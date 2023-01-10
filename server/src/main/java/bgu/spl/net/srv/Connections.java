package bgu.spl.net.srv;



public interface Connections<T> {

    boolean send(int connectionId, T msg);

    void send(String channel, T msg);

    void disconnect(int connectionId);

    boolean isConnection(int connectionId);

    void addConnection(int connectionId, ConnectionHandler<T> handler);



}

