import azure.functions as func
import logging
import os
import json
import datetime
from azure.storage.blob import BlobServiceClient

# Initializing the Function App
app = func.FunctionApp(http_auth_level=func.AuthLevel.ANONYMOUS)

@app.route(route="emqx_to_storage")
def emqx_to_storage(req: func.HttpRequest) -> func.HttpResponse:
    """
    Ghost Node: Ingests real-time Wi-Fi threat telemetry from EMQX MQTT Broker
    and persists it to Azure Blob Storage for long-term analytics.
    """
    logging.info('Ghost Node: Inbound telemetry received from EMQX.')

    try:
        # 1. Parse JSON payload
        req_body = req.get_json()
        
        # 2. Retrieve Connection String from Environment Variables
        # This keeps credentials secure and separate from the source code.
        conn_str = os.environ.get("GHOSTNODE_STORAGE_CONNECTION")
        
        if not conn_str:
            logging.error("Configuration Error: GHOSTNODE_STORAGE_CONNECTION not found.")
            return func.HttpResponse("Internal Configuration Error.", status_code=500)

        blob_service_client = BlobServiceClient.from_connection_string(conn_str)
        
        # 3. Dynamic Filename Generation (UTC-based for global standards)
        device_id = req_body.get('clientid') or req_body.get('clientId') or 'unknown_node'
        timestamp = datetime.datetime.utcnow().strftime("%Y%m%d-%H%M%S")
        filename = f"{device_id}/{timestamp}.json"
        
        # 4. Persistence Layer
        container_name = "sentineldata"
        blob_client = blob_service_client.get_blob_client(container=container_name, blob=filename)
        
        blob_client.upload_blob(json.dumps(req_body), overwrite=True)

        logging.info(f"Telemetry persisted successfully: {filename}")
        return func.HttpResponse("Success: Telemetry Ingested", status_code=200)

    except ValueError:
        return func.HttpResponse("Invalid JSON payload.", status_code=400)
    except Exception as e:
        logging.error(f"Ghost Node Runtime Error: {str(e)}")
        return func.HttpResponse("Internal Server Error.", status_code=500)