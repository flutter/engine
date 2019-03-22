#!/bin/bash

dart test/web_sdk/test/api_conform_test.dart

cd frontend_server
dart test/server_test.dart


