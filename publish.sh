#!/bin/bash
rsync -a --delete --progress releases/* root@brambasiel.be:/var/www/brambasiel.be/builds/
