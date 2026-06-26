#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "web_page.h"

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Handle root URL ("/") requests
void handleRoot(AsyncWebServerRequest *request)
{
  AsyncWebServerResponse *response = new AsyncProgmemResponse(200, "text/html", reinterpret_cast<const uint8_t *>(index_html), sizeof(index_html) - 1);
  request->send(response);
}

// Handle JSON command requests
void handleJsonCommand(AsyncWebServerRequest *request)
{
  // Check if the "json" parameter is present in the request
  if (!request->hasParam("json"))
  {
    request->send(400, "text/plain", "missing json");
    return;
  }

  // Get the JSON string from the request parameter
  String jsonCmdWebString = request->getParam("json")->value();
  DeserializationError err = deserializeJson(jsonCmdReceive, jsonCmdWebString);

  // Check for JSON deserialization errors
  if (err)
  {
    // Clear the JSON documents and send an error response
    jsonCmdReceive.clear();
    jsonInfoHttp.clear();
    request->send(400, "text/plain", err.c_str());
    return;
  }

  // Handle the JSON command
  jsonCmdReceiveHandler();
  serializeJson(jsonInfoHttp, jsonFeedbackWeb);
  request->send(200, "text/plain", jsonFeedbackWeb);
  jsonFeedbackWeb = "";
  jsonInfoHttp.clear();
  jsonCmdReceive.clear();
}

// Initialize the web server and define routes
void webCtrlServer()
{
  // Define routes for the web server
  server.on("/", HTTP_GET, handleRoot);
  server.on("/js", HTTP_GET, handleJsonCommand);

  // Start server
  server.begin();
  Serial.println("Server Starts.");
}

// Initialize the HTTP web server
void initHttpWebServer()
{
  webCtrlServer();
}

// Web server FreeRTOS task
void webServerTask(void *pvParameters) {
  // Initialize the HTTP web server
  initHttpWebServer(); 

  // Memory monitor variables
  unsigned long lastMemoryCheckTime = millis();

  // Main loop of the web server task
  for (;;) {
    // Memory monitor every 5s 
    // if (millis() - lastMemoryCheckTime >= 5000) {
    //   lastMemoryCheckTime = millis();
    //   size_t freeHeap = esp_get_free_heap_size();
    //   Serial.print("Free heap: ");
    //   Serial.println(freeHeap);
    // }
    
    vTaskDelay(pdMS_TO_TICKS(10)); // Delay to yield to other tasks
  }
  
  vTaskDelete(NULL);
}