#!/bin/bash
# setup.sh PROJECT_NAME MANUFACTURER_NAME
# Fully initializes iPlug2OOS, duplicates template, and commits.

set -e

CONFIG_FILE="config.txt"
TEMPLATE_NAME="TemplateProject"

# Use CLI args if provided, otherwise read from config.txt
if [ "$#" -eq 2 ]; then
    PROJECT_NAME=$1
    MANUFACTURER_NAME=$2
elif [ -f "$CONFIG_FILE" ]; then
    PROJECT_NAME=$(grep -E "^PROJECT_NAME=" "$CONFIG_FILE" | cut -d'=' -f2)
    MANUFACTURER_NAME=$(grep -E "^MANUFACTURER_NAME=" "$CONFIG_FILE" | cut -d'=' -f2)
else
    echo "Usage: $0 PROJECT_NAME MANUFACTURER_NAME or provide config.txt"
    exit 1
fi

if [ ! -d "$TEMPLATE_NAME" ]; then
    echo "Error: Template folder '$TEMPLATE_NAME' not found!"
    exit 1
fi

if [ -d "$PROJECT_NAME" ]; then
    echo "Removing existing project folder '$PROJECT_NAME'..."
    rm -rf "$PROJECT_NAME"
fi

echo "Duplicating template project '$TEMPLATE_NAME' to '$PROJECT_NAME'..."
export PYTHONPATH=$(pwd)   # ensure parse_config can be found
python3 duplicate.py "$TEMPLATE_NAME" "$PROJECT_NAME" "$MANUFACTURER_NAME"

echo "Cleaning up template and test folders..."
rm -rf "$TEMPLATE_NAME"
rm -rf "TestOOS"

git add "$PROJECT_NAME"
git commit -m "Created project $PROJECT_NAME for $MANUFACTURER_NAME"

echo "Setup complete! '$PROJECT_NAME' created and '$TEMPLATE_NAME' removed."
