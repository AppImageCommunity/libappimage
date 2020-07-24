#! /bin/bash

set -e

CUR_DIR=$(readlink -f $(dirname "$0"))

cd "$CUR_DIR"

VENV="$CUR_DIR"/.venv

if [ ! -d "$VENV" ] || [ ! -e "$VENV"/bin/activate ]; then
    echo $(tput bold)$(tput setaf 2)"Creating new virtual environment in $VENV"$(tput sgr0)
    PYTHON=python3
    which python3.6 &>/dev/null && PYTHON=python3.6
    "$PYTHON" -m venv "$VENV"
fi

source "$VENV"/bin/activate

# this snippet should allow us to call pip install only if the requirements file has been touched
if [ ! -f "$VENV"/requirements.txt ] || [ $(sha256sum requirements.txt | cut -d' ' -f1) != $(sha256sum "$VENV"/requirements.txt | cut -d' ' -f1) ]; then
    echo $(tput bold)$(tput setaf 2)"Requirements updated, reinstalling"$(tput sgr0)
    cp requirements.txt "$VENV"/requirements.txt
    pip install -U -r "$VENV"/requirements.txt
fi

make "$@"
