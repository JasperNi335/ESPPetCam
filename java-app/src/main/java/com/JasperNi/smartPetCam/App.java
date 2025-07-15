package com.JasperNi.smartPetCam;

import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import com.google.gson.Gson;

import java.util.Base64;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URI;
import java.net.URL;
import java.net.InetSocketAddress;
import java.io.File;

public class App {
    static final int STREAM_BUFFER_SIZE = 8192;
    public static void main(String[] args) throws IOException{
        HttpServer server = HttpServer.create(new InetSocketAddress(8080), 0);

        server.createContext("/status", new StatusHandler());
        server.createContext("/upload", new UploadHandler());
        server.createContext("/view_image", new ImageHandler());
        server.createContext("/stream", new StreamHandler());

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

                // decode image from base64 and saves it as a picture
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
            // access saved image from request
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

    static class StreamHandler implements HttpHandler{
        public void handle(HttpExchange exchange) throws IOException{
            String urlString = "http://" + Keys.Key.IP.getKeyName() + "/stream";

            System.out.println("URL is: " + urlString);

            URI uri = URI.create(urlString);
            URL url = uri.toURL();

            HttpURLConnection connection = (HttpURLConnection) url.openConnection();
            connection.setRequestMethod("GET");
            connection.setConnectTimeout(5000);
            connection.setReadTimeout(0);

            InputStream cameraStream = null;
            OutputStream os = exchange.getResponseBody();

            try {
                cameraStream = connection.getInputStream();

                exchange.getResponseHeaders().set("Content-Type", "multipart/x-mixed-replace; boundary=frame");
                exchange.sendResponseHeaders(200, 0);

                byte[] buffer = new byte[STREAM_BUFFER_SIZE];
                int bytesRead;

                while ((bytesRead = cameraStream.read(buffer)) != -1){
                    os.write(buffer, 0, bytesRead);
                    os.flush();
                }
            } catch (IOException e){
                System.err.println("StreamHandler error: " + e.getMessage());
            }finally {
                if (cameraStream != null) {
                    cameraStream.close();
                }
                os.close();
            }
        }
    }
}
