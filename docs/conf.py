# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
import os
import sys
sys.path.insert(0, os.path.abspath('.'))


# -- Project information -----------------------------------------------------

project = 'DYAD'
copyright = """Copyright 2021 Lawrence Livermore National Security, LLC. SPDX-License-Identifier: LGPL-3.0"""
author = 'This page is maintained by the <a href="https://github.com/flux-framework/dyad/graphs/contributors">contributors to DYAD</a>'


# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [
    "sphinx.ext.autosectionlabel",
    'myst_parser',
]

# Try to load rst2pdf for PDF output
try:
    import rst2pdf
    extensions.append('rst2pdf.pdfbuilder')
    pdf_stylesheets = ['sphinx', 'a4']
except (ImportError, Exception):
    pass

# This line explicitly tells Sphinx which parser to use for each extension
source_suffix = {
    '.rst': 'restructuredtext',
    '.md': 'markdown',
}

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = ['_build', '_fragments', 'Thumbs.db', '.DS_Store', 'venv', '.venv']

raw_enabled = True

# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = 'sphinx_rtd_theme'

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ['_static']
html_logo = "_static/logo/dyad_logo_sky_blue.svg"
html_css_files = ["logo/logo.css"]

# -- Options for Breathe  ---------------------------------------------------

# Conditionally enable Breathe to bridge Doxygen XML into Sphinx

try:
    import breathe
    import os
    xml_path = os.path.join(os.path.dirname(__file__), "doxygen/xml")
    print(f"Breathe found. XML path: {xml_path}")
    print(f"XML exists: {os.path.exists(os.path.join(xml_path, 'index.xml'))}")
    if os.path.exists(os.path.join(xml_path, "index.xml")):
        extensions.append('breathe')
        breathe_projects = {"dyad": xml_path}
        breathe_default_project = "dyad"
        tags.add('has_breathe')
        print("Breathe enabled.")
    else:
        print("Warning: Doxygen XML not found, skipping API doc generation.")
except ImportError:
    print("Breathe not installed, skipping API doc generation.")
