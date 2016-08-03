# Subdirectories
SUBDIRS = src .
if HAVE_CHECK
SUBDIRS += tests
endif

# Copy extra distribution files
docdir = $(datadir)/doc/$(PACKAGE)
doc_DATA = ChangeLog NEWS README COPYING

# Copy public domain examples
examples = examples/*
examplesdir = $(datadir)/doc/@PACKAGE@/roms
examples_DATA = $(examples)

# Extra files that should come with dist (like ROMs)
EXTRA_DIST = $(examples) debian/*
