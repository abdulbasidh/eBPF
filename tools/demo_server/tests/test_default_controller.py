# coding: utf-8

import pytest
import json
from aiohttp import web

from openapi_server.models.inline_response200 import InlineResponse200


async def test_big_payload_get(client):
    """Test case for big_payload_get

    Accepts an arbitrary, or empty request body, and returns a JSON document that includes 10MB of raw data encoded as base64
    """
    body = None
    headers = { 
        'Accept': 'application/json',
        'Content-Type': 'application/json',
    }
    response = await client.request(
        method='GET',
        path='/big/payload',
        headers=headers,
        json=body,
        )
    assert response.status == 200, 'Response body is : ' + (await response.read()).decode('utf-8')

