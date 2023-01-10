package bgu.spl.net.srv;

import bgu.spl.net.impl.stomp.StompFrame;
import bgu.spl.net.impl.stomp.serverDataBase;
import bgu.spl.net.impl.stomp.StompFrame.Frame;
import bgu.spl.net.impl.stomp.serverDataBase.User;

import java.util.List;
import java.util.concurrent.ConcurrentHashMap;

public class ConnectionsImpl<T> implements Connections<T> {
    private static boolean isEx = true;

    private int messageID = 7;

    private ConcurrentHashMap<Integer, ConnectionHandler> connections;
    serverDataBase dataBase = serverDataBase.getInstance();

    public ConnectionsImpl() {
        connections = new ConcurrentHashMap<>();
    }

    @Override
    public void addConnection(int connectionId, ConnectionHandler<T> handler) {
        this.connections.put(connectionId, handler);

    }

    @Override
    public boolean isConnection(int connectionId) {
        return connections.containsKey(connectionId);
    }

    @Override
    public boolean send(int connectionId, T msg) {
        isEx = true;
        connections.get(connectionId).send(msg);
        return isEx;
    }

    @Override
    public void send(String channel, T msg) {

        List<User> temp = dataBase.getSubscribers(channel);
        if (temp != null && temp.size() != 0) {
            for (User user : temp) {
                if(msg instanceof Frame){
                    ((Frame) msg).setSubscription(user.getSubscriptionId(channel));
                    ((Frame) msg).setMessageId(String.valueOf(messageID++));
                }
                String output = StompFrame.encode((Frame) msg);
                connections.get(user.getConnectionId()).send(output);
            }
        }
    }

    @Override
    public void disconnect(int connectionId) {
        connections.remove(connectionId);
    }

    public static void setEx(boolean ex) {
        isEx = ex;
    }

}
