package e.devin.ecgclient;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;

import com.jjoe64.graphview.GraphView;
import com.jjoe64.graphview.Viewport;
import com.jjoe64.graphview.series.DataPoint;
import com.jjoe64.graphview.series.LineGraphSeries;
import com.jjoe64.graphview.series.PointsGraphSeries;

import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.Socket;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.Arrays;

public class MainActivity extends AppCompatActivity {

    // declaring variables
    private PointsGraphSeries<DataPoint> mSeries;
    private double lastX = 0;
    public static final String TAG =
            e.devin.ecgclient.MainActivity.class.getSimpleName();
    public Socket socket;
    public static boolean changeChannel = false;
    public static boolean changeFilter = false;
    TextView portInput;
    TextView addressInput;
    TextView channelInput;
    Button disconnect;
    Button channelSelect;
    Button buttonConnect;
    Button filterSelect;
    Thread thread;
    GraphView ecgOutput;
    Viewport viewport;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // assigning variables used by xml
        buttonConnect = findViewById(R.id.connectButton);
        disconnect = findViewById(R.id.disconnectButton);
        channelSelect = findViewById(R.id.channelButton);
        filterSelect = findViewById(R.id.filterButton);
        portInput = findViewById(R.id.inputPort);
        addressInput = findViewById(R.id.inputAddress);
        channelInput = findViewById(R.id.inputChannel);
        ecgOutput = findViewById(R.id.graph);
        mSeries = new PointsGraphSeries<>();
        ecgOutput.addSeries(mSeries);

        filterSelect.setVisibility(View.GONE);
        disconnect.setVisibility(View.GONE);
        channelSelect.setVisibility(View.GONE);
        channelInput.setVisibility(View.GONE);

        // setup viewport for optimal graphing behavior
        viewport = ecgOutput.getViewport();
        viewport.setYAxisBoundsManual(true);
        viewport.setXAxisBoundsManual(true);
        viewport.setMinX(0);
        viewport.setMaxX(3);
        viewport.setMinY(0);
        viewport.setMaxY(4200);
        viewport.setScrollable(true);
        mSeries.setSize((float) 3);


        buttonConnect.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View view) {
                disconnect.setVisibility(View.VISIBLE);
                channelSelect.setVisibility(View.VISIBLE);
                channelInput.setVisibility(View.VISIBLE);
                filterSelect.setVisibility(View.VISIBLE);
                portInput.setVisibility(View.GONE);
                addressInput.setVisibility(View.GONE);
                buttonConnect.setVisibility(View.GONE);
                RunClient clientThread = new
                        RunClient(addressInput.getText().toString(),
                        Integer.parseInt(portInput.getText().toString()));
                thread = new Thread(clientThread);
                thread.start();
                while(socket == null){
                    socket = clientThread.getSocket();
                    Log.i(TAG, "Connected" + socket);
                }
            }
        });

        channelSelect.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View view) {
                changeChannel = true;
            }
        });

        filterSelect.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View view) {
                changeFilter = true;
            }
        });

        disconnect.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View view) {
                thread.interrupt();
                disconnect.setVisibility(View.GONE);
                channelSelect.setVisibility(View.GONE);
                channelInput.setVisibility(View.GONE);
                filterSelect.setVisibility(View.GONE);
                portInput.setVisibility(View.VISIBLE);
                addressInput.setVisibility(View.VISIBLE);
                buttonConnect.setVisibility(View.VISIBLE);
            }
        });

    }

    public void updateMessage(final String message){
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                int decimalValue = Integer.parseInt(message, 2);
                lastX += .0008;
                mSeries.appendData(new DataPoint(lastX, decimalValue),
                        true, 10000);
            }
        });

    }

    class RunClient implements Runnable{

        private static final int CHANNELBIT = 6;
        private static final int FILTERBIT = 7;
        private Socket socket;
        private BufferedReader input;
        private DataOutputStream out;
        private String serverAddress;
        private int serverPort, clientOut;
        private byte[] b;
        private ByteBuffer buff;

        RunClient(String address, int port) {
            serverAddress = address;
            serverPort = port;
        }

        @Override
        public void run(){

            try{

                Log.i(TAG, "Connecting to server...");

                socket = new Socket(serverAddress, serverPort);
                String message;


                Log.i(TAG, "Connected to server..." + socket);

                this.input = new BufferedReader(new
                        InputStreamReader(socket.getInputStream()));

                this.out = new DataOutputStream(socket.getOutputStream());

                while(!Thread.currentThread().isInterrupted() && !socket.isClosed()){

                    message = input.readLine();

                    if(message != null) {
                        updateMessage(message);
                    }

                    if(changeChannel || changeFilter){
                        this.clientOut = Integer.parseInt(channelInput.getText().toString());
                        this.buff = ByteBuffer.allocate(4);
                        //this.b = buff.order(ByteOrder.BIG_ENDIAN).putInt(clientOut).array();
                        if(changeChannel){
                            clientOut = clientOut | (1 << CHANNELBIT);
                            changeChannel = false;
                            Log.i(TAG, "Channel Num " + clientOut);
                        }else{
                            clientOut = clientOut | (1 << FILTERBIT);
                            changeFilter = false;
                            Log.i(TAG, "Filter Num " + clientOut);
                        }
                        out.write(clientOut);
                        out.flush();
                    }
                }
                socket.close();
            } catch (UnknownHostException e){
                throw new RuntimeException(e.getMessage());
            } catch (IOException e){
                throw new RuntimeException(e.getMessage());
            } catch (IllegalArgumentException e){
                throw new RuntimeException(e.getMessage());
            }
        }

        public Socket getSocket() {
            return socket;
        }
    }
}