package com.st.logger.util;

import android.annotation.SuppressLint;
import android.app.Application;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.os.Environment;
import android.os.IBinder;
import android.support.annotation.Nullable;
import android.support.v4.util.SimpleArrayMap;
import android.util.Log;

import com.st.logger.core.connectionInfo;
import com.st.logger.core.connectionInfoList;
import com.st.logger.core.dmesgInfo;
import com.st.logger.core.processInfo;
import com.st.logger.core.processInfoList;
import com.st.logger.core.dmesgInfoList;
import com.st.logger.ipc.IpcService;
import com.st.logger.ipc.IpcService.ipcClientListener;
import com.st.logger.ipc.ipcCategory;
import com.st.logger.ipc.ipcData;
import com.st.logger.ipc.ipcMessage;
import com.st.logger.settings.Settings;

import java.io.FileWriter;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class SoakService extends Service implements ipcClientListener {

    // ipc client
    private IpcService ipcService;
    private boolean ipcStop = false;

    // data
    private ArrayList<connectionInfo> data = new ArrayList<>();
    private ArrayList<dmesgInfo> kdata = new ArrayList<>();
    private Settings settings = null;

    private ProcessUtil infoHelper = null;

    @SuppressLint("UseSparseArrays")
    private SimpleArrayMap<Integer, String> map = new SimpleArrayMap<>();
    private Map<String, String> dict = new HashMap<>();


    // stop or start
    private boolean stopUpdate = false;

    private PackageManager pm;
    private List<ApplicationInfo> pkgList;
    private List<PackageInfo> appList;
    private FileWriter fileWriter;

    private DateFormat dateFormat;
    private Calendar calendar;

    public void onCreate() {

        settings = Settings.getInstance(this.getApplicationContext());
        infoHelper = ProcessUtil.getInstance(this.getApplicationContext(),
                true);

        pm = getPackageManager();
        pkgList = pm.getInstalledApplications(0);
        appList = pm.getInstalledPackages(PackageManager.GET_PERMISSIONS);
        try {
            Date currTime = Calendar.getInstance().getTime();
            String fileName = Environment.getExternalStoragePublicDirectory
                    (Environment.DIRECTORY_DOWNLOADS).getAbsolutePath() + "/" + currTime + ".txt";
            fileWriter = new FileWriter(fileName, true);
            fileWriter.write("start time: " + currTime + "\n");
            fileWriter.flush();
        } catch (IOException e) {
            //Log.d("SOAK", "write failed...");
            e.printStackTrace();
        }

        dateFormat = new SimpleDateFormat("yyyy/MM/dd HH:mm:ss.SSS");


        //System.out.println(dateFormat.format(calendar.getTime())); //2014/08/06 16:00:22
    }

    private String getPackageName(String localIP, int uid) {
        for (ApplicationInfo appInfo : pkgList) {
            if (uid == appInfo.uid) {
                dict.put(localIP, appInfo.packageName);
                return appInfo.packageName;
            }
        }
        if (dict.containsKey(localIP)) {
            return dict.get(localIP);
        }
        if (uid == 0) {
            return "system";
        }
        return uid + "(UID)";
    }

    private String getAppName(String localIP, int uid) {
        String pkgName = getPackageName(localIP, uid);
        for (PackageInfo pkgInfo : appList) {
            ApplicationInfo appInfo = pkgInfo.applicationInfo;
            if (pkgInfo.packageName.equals(pkgName)) {
                return "," + appInfo.loadLabel(pm);
            }
        }
        return "";
    }

    private String getDmesgInfoString(dmesgInfo item) {
        StringBuilder logLine = new StringBuilder();
        logLine.append(UserInterfaceUtil.getDmesgLevel(item.level()));
        logLine.append(item.seconds() + " " + item.message());

        try {
            fileWriter.write(logLine.toString() + "\n");
            fileWriter.flush();
        } catch (IOException e) {
            //Log.d("SOAK", "write failed...");
            e.printStackTrace();
        }

        return logLine.toString();
    }

    private String getConnectionInfoString(connectionInfo item) {
        StringBuilder logLine = new StringBuilder();

        // sungmin2.lee add time stamp
        calendar = Calendar.getInstance();
        logLine.append(dateFormat.format(calendar.getTime()) + " ");

        // prepare main information
        logLine.append(UserInterfaceUtil.getConnectionType(item.type()));
        logLine.append(",");

        String localIP = UserInterfaceUtil.convertToIPv4(item.localIP(), item.localPort());
        logLine.append(localIP);
        logLine.append(",");

        String remoteIP = UserInterfaceUtil.convertToIPv4(item.remoteIP(), item.remotePort());
        logLine.append(remoteIP);
        logLine.append(",");

        logLine.append(UserInterfaceUtil.getConnectionStatus(item.status()));
        logLine.append(",");

        //if (item.uid() == 0)
        //    logLine.append("System");
        //else
        //if (map.containsKey(item.uid()))
        //    logLine.append(infoHelper.getPackageName(map.get(item.uid())));
        //else
        //    logLine.append(item.uid() + "(UID)");
        //logLine.append("\n");

        logLine.append(getPackageName(localIP, item.uid()) + getAppName(localIP, item.uid()));

        try {
            fileWriter.write(logLine.toString() + "\n");
            fileWriter.flush();
        } catch (IOException e) {
            //Log.d("SOAK", "write failed...");
            e.printStackTrace();
        }

        return logLine.toString();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        try {
            fileWriter.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    @Override
    public void onRecvData(byte[] result) {

        // check
        if (ipcStop == true)
            return;

        if (stopUpdate == true || result == null) {
            byte newCommand[] = {ipcCategory.CONNECTION, ipcCategory.PROCESS, ipcCategory.DMESG};
            ipcService.addRequest(newCommand, settings.getInterval(), this);
            return;
        }

        // clean up
        while (!data.isEmpty())
            data.remove(0);
        data.clear();

        map.clear();

        // convert data
        ipcMessage resultMessage = ipcMessage.getRootAsipcMessage(ByteBuffer.wrap(result));
        try {
            for (int index = 0; index < resultMessage.dataLength(); index++) {

                ipcData rawData = resultMessage.data(index);

                // prepare mapping table
                if (rawData.category() == ipcCategory.PROCESS)
                    extractProcessInfo(rawData);
                else if (rawData.category() == ipcCategory.CONNECTION)
                    extractConnectionInfo(rawData);
                else if (rawData.category() == ipcCategory.DMESG)
                    extractDmesgInfo(rawData);

            }
        } catch (Exception e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }

        //TODO: output IP mapping.
        for (connectionInfo cnInfo : data) {
            Log.v("SOAK", getConnectionInfoString(cnInfo));
        }
        Log.v("SOAK", "\n");
        for (dmesgInfo dInfo: kdata) {
            Log.v("SOAK-d", getDmesgInfoString(dInfo));
        }
        Log.v("SOAK-d", "\n");

        // send command again
        byte newCommand[] = {ipcCategory.CONNECTION, ipcCategory.PROCESS, ipcCategory.DMESG};
        ipcService.addRequest(newCommand, settings.getInterval(), this);
    }


    private void extractConnectionInfo(ipcData rawData) {
        // process connectionInfo
        connectionInfoList list = connectionInfoList.getRootAsconnectionInfoList(rawData.payloadAsByteBuffer().asReadOnlyBuffer());
        for (int count = 0; count < list.listLength(); count++) {
            connectionInfo cnInfo = list.list(count);
            data.add(cnInfo);
            //Log.v("SOAK", getConnectionInfoString(cnInfo));
        }
    }

    private void extractProcessInfo(ipcData rawData) {
        processInfoList list = processInfoList.getRootAsprocessInfoList(rawData.payloadAsByteBuffer().asReadOnlyBuffer());
        for (int count = 0; count < list.listLength(); count++) {
            processInfo psInfo = list.list(count);
            if (!infoHelper.checkPackageInformation(psInfo.name())) {
                infoHelper.doCacheInfo(psInfo.uid(), psInfo.owner(), psInfo.name());
            }
            map.put(psInfo.uid(), psInfo.name());
        }
    }

    private void extractDmesgInfo(ipcData rawData) {
        dmesgInfoList list = dmesgInfoList.getRootAsdmesgInfoList(rawData.payloadAsByteBuffer().asReadOnlyBuffer());
        for (int count = 0; count < list.listLength(); count++) {
            dmesgInfo dInfo = list.list(count);
            kdata.add(dInfo);
        }
    }

    /**
     * Return the communication channel to the service.  May return null if
     * clients can not bind to the service.  The returned
     * {@link IBinder} is usually for a complex interface
     * that has been <a href="{@docRoot}guide/components/aidl.html">described using
     * aidl</a>.
     * <p/>
     * <p><em>Note that unlike other application components, calls on to the
     * IBinder interface returned here may not happen on the main thread
     * of the process</em>.  More information about the main thread can be found in
     * <a href="{@docRoot}guide/topics/fundamentals/processes-and-threads.html">Processes and
     * Threads</a>.</p>
     *
     * @param intent The Intent that was used to bind to this service,
     *               as given to {@link Context#bindService
     *               Context.bindService}.  Note that any extras that were included with
     *               the Intent at that point will <em>not</em> be seen here.
     * @return Return an IBinder through which clients can call on to the
     * service.
     */
    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        // The service is starting, due to a call to startService()
        byte newCommand[] = {ipcCategory.CONNECTION, ipcCategory.PROCESS};

        IpcService.Initialize(this);
        ipcService = IpcService.getInstance();
        ipcService.addRequest(newCommand, 0, this);

        return START_STICKY;
    }

}
