from aiohttp import web
import json

# from openapi_server.models.inline_response200 import InlineResponse200
# from openapi_server import util


async def hello_world_get(request: web.Request, body=None) -> web.Response:
    ohai = {
        "Message": "Hello World!",
    }
    return web.Response(status=200, content_type="application/json", body=json.dumps(ohai, indent=2))
