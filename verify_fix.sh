#!/bin/bash
set -e

echo "Logging in..."
LOGIN_RESP=$(curl -s -X POST -H "Content-Type: application/json" -d '{"username":"admin","password":"admin"}' http://localhost:8081/api/login)
TOKEN=$(echo $LOGIN_RESP | grep -o '"token":"[^"]*' | cut -d'"' -f4)
echo "Token: $TOKEN"

echo "Creating task..."
CREATE_RESP=$(curl -s -X POST -H "Content-Type: application/json" -H "Authorization: Bearer $TOKEN" -d '{"title":"Test Task","description":"Test Description","deadlineDays":5}' http://localhost:8081/api/tasks)
TASK_ID=$(echo $CREATE_RESP | grep -o '"id":[0-9]*' | cut -d':' -f2)
echo "Created Task ID: $TASK_ID"

echo "Assigning task to user 3 (dev1)..."
ASSIGN_RESP=$(curl -s -X PUT -H "Content-Type: application/json" -H "Authorization: Bearer $TOKEN" -d '{"assigneeId":3}' http://localhost:8081/api/tasks/$TASK_ID/assign)
echo "Assign Response: $ASSIGN_RESP"
