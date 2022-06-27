#!/bin/bash -e
cd "$(dirname "${BASH_SOURCE[0]}")"
openapi-generator-cli generate -g python-aiohttp -c python-aiohttp.conf.yml \
  -i openapi.yaml -o ../
