# Minimal makefile for Sphinx documentation
#

# You can set these variables from the command line, and also
# from the environment for the first two.
SPHINXOPTS    ?=
SPHINXBUILD   ?= sphinx-build
SOURCEDIR     = .
BUILDDIR      = _build
DOXYFILE      = Doxyfile

# Put it first so that "make" without argument is like "make help".
help:
	@$(SPHINXBUILD) -M help "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)

.PHONY: help Makefile doxygen html latex man epub clean

doxygen:
	@if command -v doxygen > /dev/null 2>&1; then \
	    echo "Running Doxygen..."; \
	    doxygen $(DOXYFILE); \
	else \
	    echo "Warning: doxygen not found, skipping API doc generation."; \
	fi

clean:
	@$(SPHINXBUILD) -M clean "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)
	@echo "Removing Doxygen output..."
	@rm -rf doxygen/
	@rmdir _build

# Run Doxygen before any Sphinx target
html latex man epub: doxygen
	@$(SPHINXBUILD) -M $@ "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)

# Catch-all target: route all unknown targets to Sphinx using the new
# "make mode" option.  $(O) is meant as a shortcut for $(SPHINXOPTS).
HANDLED_TARGETS = help clean doxygen
%: Makefile
	@$(if $(filter $@,$(HANDLED_TARGETS)),true,$(SPHINXBUILD) -M $@ "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O))
