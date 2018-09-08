#!/bin/bash

if [ "$TRAVIS_EVENT_TYPE" != "cron" ] ; then
  echo "Not running from cron, exiting"
  exit 0
fi

# MAJOR version is incremented in case of incompatible API changes.
# 1 = klik, 2 = klik2, 3 = AppImage
# We expect NOT to make incompatible API changes
# in this library, or else a discussion must have taken place
# and sufficient (think years, not days) warning time must have
# passed. In this case, we must manually increase this number
MAJOR=3

# MINOR version is incremented when adding functionality 
# in a backwards-compatible manner.
# We assume to have new features every release, and to signify that this
# is a time-based release we take the reverse date as the MINOR version
MINOR=$(date +%Y%m%d)

# PATCH version would be incremented if we would make
# backwards-compatible bug fixes (without adding new features).
# Due to the nature of our continuous build philosophy in which master
# is supposed to always be in a working state (proven by test automation)
# and which is the only branch that is actively maintained, 
# we don't usually provide backwards-compatible bug fixes. Should
# exceptions be necessary, a patch release would need to be made manually
PATCH=0

# MAJOR.MINOR.PATCH as per https://semver.org/
VERSION=$MAJOR.$MINOR.$PATCH

create_tag() {
	echo "git pull..."
	git pull
	echo "git push..."
	git push
	echo "Tagging $VERSION at $COMMIT..."
	git tag $VERSION $COMMIT
	git push origin --tags
}

create_tag
