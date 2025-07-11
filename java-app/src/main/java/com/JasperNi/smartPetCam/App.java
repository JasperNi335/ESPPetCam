package com.JasperNi.smartPetCam;

import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import com.google.gson.Gson;

import java.util.Base64;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.io.File;


public class App {
    public static void main(String[] args) throws IOException{
        HttpServer server = HttpServer.create(new InetSocketAddress(8080), 0);

        server.createContext("/status", new StatusHandler());
        server.createContext("/upload", new UploadHandler());
        server.createContext("/view_image", new ImageHandler());

        server.setExecutor(null); // default executor
        server.start();

        System.out.println("Server running on http://localhost:8080");
    }

    static class StatusHandler implements HttpHandler {
        public void handle(HttpExchange exchange) throws IOException{
            String response = "online";
            exchange.getResponseHeaders().add("Content-Type", "text/plain");
            exchange.sendResponseHeaders(200, response.length());

            OutputStream os = exchange.getResponseBody();
            os.write(response.getBytes());
            os.flush();
            os.close();
        }
    }

    static class UploadHandler implements HttpHandler{
        public void handle(HttpExchange exchange) throws IOException{
            if ("POST".equals(exchange.getRequestMethod())){
                String recieved = new String(exchange.getRequestBody().readAllBytes());

                // manage recieve JSON here
                Gson gson = new Gson();
                ImageJSONClass recievedImage = gson.fromJson(recieved, ImageJSONClass.class);

                // decode image from base64
                byte[] imageBytes = Base64.getDecoder().decode(recievedImage.getImage());
                try (FileOutputStream fos = new FileOutputStream("recieved.jpg")){
                    fos.write(imageBytes);
                    fos.flush();
                    System.out.println("Image saved as received.jpg");
                }catch(IOException e){
                    System.err.println("Failed to get image, Error: " + e.getMessage());
                }

                String response = "Image Recieved and Saved";
                exchange.sendResponseHeaders(200, response.length());
                OutputStream os = exchange.getResponseBody();
                os.write(response.getBytes());
                os.flush();
                os.close();
            }else{
                exchange.sendResponseHeaders(405, -1);
            }
        }   
    }

    static class ImageHandler implements HttpHandler{
        public void handle(HttpExchange exchange) throws IOException{
            // _______________________________________________
            //
            // DO LATER: MAKE THE IMAGE AUTO REFRESH/ IMPLEMENT VIDEO
            // _______________________________________________
            File imageFile = new File("recieved.jpg");

            OutputStream os = exchange.getResponseBody();

            if (!imageFile.exists()){
                String response = "No Image found";
                exchange.sendResponseHeaders(404, response.length());

                os.write(response.getBytes());
                os.flush();
                os.close();
                return;
            }

            byte[] image = java.nio.file.Files.readAllBytes(imageFile.toPath());

            exchange.getResponseHeaders().add("Content-Type", "image/jpeg");
            exchange.sendResponseHeaders(200, image.length);

            os.write(image);
            os.flush();
            os.close();
            return;
        }
    }
}
