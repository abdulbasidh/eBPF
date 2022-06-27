from typing import List, Dict
from aiohttp import web

from openapi_server import util


async def hello_world_get(request: web.Request, body=None) -> web.Response:
    """Accepts an arbitrary, or empty request body, and returns a short JSON document

    

    :param body: 
    :type body: 

    """
    return web.Response(status=200)
