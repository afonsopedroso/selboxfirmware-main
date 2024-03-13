from fastapi import Request
from fastapi import FastAPI
from fastapi import Form
from fastapi import BackgroundTasks
from fastapi.responses import HTMLResponse
from fastapi.staticfiles import StaticFiles
from fastapi.templating import Jinja2Templates
from fastapi.responses import JSONResponse

from library import network_connector

# Initialize fast API
app = FastAPI()
app.mount("/static", StaticFiles(directory="static"), name="static")
templates = Jinja2Templates(directory="templates")


@app.get("/", response_class=HTMLResponse)
async def root(request: Request):
    # Default endpoint to load view
    return templates.TemplateResponse("set_wifi.html", {"request": request})


@app.post("/")
async def set(request: Request, background_tasks: BackgroundTasks, ssid: str = Form(...), password: str = Form(...)):
    # Endpoint to set ssid and password on a background task
    network_connector.write_network_value_file(key="reconnect", value=0)
    background_tasks.add_task(network_connector.connect_to_wifi, ssid=ssid, password=password)
    return templates.TemplateResponse("set_wifi.html", {"request": request, "response": "1"})


@app.get("/wifi")
async def wifi_list():
    # End point to get wifi network list
    wifi_list = network_connector.get_all_wifi_network()
    return JSONResponse(content=wifi_list)


@app.get("/check")
async def check_network(background_tasks: BackgroundTasks):
    # Endpoint to check network status
    background_tasks.add_task(network_connector.network_check)
    return JSONResponse(content={"success": True})


@app.get("/reconnect")
async def reconnect_wifi(background_tasks: BackgroundTasks):
    # Endpoint to reconnect to wifi on loosing network
    background_tasks.add_task(network_connector.reconnect_to_wifi)
    return JSONResponse(content={"success": True})


@app.get("/create")
async def create_hotspot(background_tasks: BackgroundTasks):
    # Endpoint to create hotspot
    background_tasks.add_task(network_connector.create_hotspot)
    return JSONResponse(content={"success": True})
