Write-Host "Running at $PWD, (it's docker so it will probably crash)"
docker run -it --mount type=bind,source=$PWD,target=/app abf3830e1398
