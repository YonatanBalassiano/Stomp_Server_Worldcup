package bgu.spl.net.impl.stomp;
import bgu.spl.net.srv.Server;

public class StompServer {

    public static void main(String[] args) {
        int port = Integer.parseInt(args[0]);
        boolean serverType = args[1].equals("tpc") ? true : false;
        // you can use any server... 
        if(serverType){
            System.out.println("Starting TPC server");
        Server.threadPerClient(
                port, //port
                () -> new StompMessagingProtocolImpl(), //protocol factory
                LineMessageEncoderDecoder::new //message encoder decoder factory
        ).serve();
        }
        else{
            System.out.println("Starting Reactor server");
            Server.reactor(
                Runtime.getRuntime().availableProcessors(),
                port, //port
                () -> new StompMessagingProtocolImpl(), //protocol factory
                LineMessageEncoderDecoder::new //message encoder decoder factory
        ).serve();
        }

    }
}
