#!/usr/bin/env bash

set -ex

virtualenv -p python3 docbuildenv
. ./docbuildenv/bin/activate
pip install -r requirements.txt
make html
