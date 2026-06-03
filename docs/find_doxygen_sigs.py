#!/usr/bin/env python3
"""
Script to find function signatures in Doxygen XML and print the
corresponding Breathe doxygenfunction directives.
"""

import sys
import os
import xml.etree.ElementTree as ET

def find_function_signatures(xml_dir, function_names):
    """Search Doxygen XML files for function signatures."""
    results = {}
    
    for xml_file in os.listdir(xml_dir):
        if not xml_file.endswith('.xml') or xml_file == 'index.xml':
            continue
        
        xml_path = os.path.join(xml_dir, xml_file)
        try:
            tree = ET.parse(xml_path)
            root = tree.getroot()
        except ET.ParseError:
            continue
        
        for member in root.iter('memberdef'):
            kind = member.get('kind')
            if kind != 'function':
                continue
            
            name_elem = member.find('name')
            if name_elem is None:
                continue
            
            name = name_elem.text
            if name not in function_names:
                continue
            
            argsstring_elem = member.find('argsstring')
            if argsstring_elem is None:
                continue
            
            argsstring = argsstring_elem.text or ''
            
            if name not in results:
                results[name] = []
            results[name].append({
                'file': xml_file.replace('_8c.xml', '.c')
                                .replace('_8h.xml', '.h')
                                .replace('_8cpp.xml', '.cpp')
                                .replace('__', '_'),
                'argsstring': argsstring,
            })
    
    return results

def main():
    if len(sys.argv) < 2:
        xml_dir = 'docs/doxygen/xml'
    else:
        xml_dir = sys.argv[1]
    
    if not os.path.isdir(xml_dir):
        print(f"Error: XML directory not found: {xml_dir}")
        sys.exit(1)
    
    # Functions to look up
    function_names = [
        # core
        'dyad_fetch_metadata',
        'dyad_kvs_commit',
        'publish_via_flux',
        'dyad_cons_store',
        # utility
        'mkpath',
        'get_stat',
        'sync_containing_dir',
        'dyad_sync_directory',
        'cmp_canonical_path_prefix',
        'extract_user_path',
        'concat_str',
        'hash_str',
        'hash_path_prefix',
        'get_file_size',
        'get_path',
        'read_all',
        'write_all',
        'is_fd_dir',
        'gen_path_key',
        'MurmurHash3_x64_128',
        'mkdir_as_needed',
    ]
    
    results = find_function_signatures(xml_dir, function_names)
    
    print("# Paste the following into your .rst files:\n")
    for name in function_names:
        if name not in results:
            print(f"# WARNING: '{name}' not found in XML")
            print(f".. doxygenfunction:: {name}")
            print(f"   :project: dyad\n")
        else:
            for match in results[name]:
                print(f".. doxygenfunction:: {name}{match['argsstring']}")
                print(f"   :project: dyad")
                print(f"   # from: {match['file']}\n")

if __name__ == '__main__':
    main()
