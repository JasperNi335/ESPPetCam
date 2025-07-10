package com.JasperNi.smartPetCam;

import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;

import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;

public class App {
    public static void main(String[] args) throws IOException{
        HttpServer server = HttpServer.create(new InetSocketAddress(8080), 0);

        server.createContext("/status", new StatusHandler());
        server.createContext("/upload", new UploadHandler());

        server.setExecutor(null); // default executor
        server.start();

        System.out.println("Server running on http://localhost:8080");
    }

    static class StatusHandler implements HttpHandler {
        public void handle(HttpExchange exchange) throws IOException{
            String response = "OK";
            exchange.sendResponseHeaders(200, response.length());

            OutputStream os = exchange.getResponseBody();
            os.write(response.getBytes());
            os.close();
        }
    }

    static class UploadHandler implements HttpHandler{
        public void handle(HttpExchange exchange) throws IOException{
            if ("Post".equals(exchange.getRequestMethod())){
                String recieved = new String(exchange.getRequestBody().readAllBytes());
                System.out.println("Recieved from ESP32: " + recieved);

                String response = "Recieved";
                OutputStream os = exchange.getResponseBody();
                os.write(response.getBytes());
                os.close();
            }else{
                exchange.sendResponseHeaders(405, -1);
            }
        }   
    }
}
