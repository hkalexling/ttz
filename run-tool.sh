#! /bin/bash

set -e

# check python3 exists
if ! command -v python3 &> /dev/null
then
	echo "python3 not found"
	exit
fi

# check venv exists
if [ ! -d "venv" ]
then
	echo "Creating virtual environment"
	python3 -m venv venv
fi

# activate venv
source venv/bin/activate
pip install -r requirements.txt > /dev/null

# input must exist
if [ -z "$1" ] || [ ! -f "tools/$1.py" ]
then
	echo "Usage: ./run-tool.sh <tool-name>"
	exit
fi

# run tool
python3 tools/$1.py
