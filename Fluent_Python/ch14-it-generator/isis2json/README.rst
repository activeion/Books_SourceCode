isis2json.py
============

This directory contains a copy of the ``isis2json.py`` script, with
minimal dependencies, just to allow the O'Reilly Atlas toolchain to
render the listing of the script in appendix A of the book.

If you want to use or contribute to this script, please get the full
source code with all dependencies from the main ``isis2json``
repository:

https://github.com/fluentpython/isis2json

debug it:

$ pudb3 ./isis2json.py cds.iso -c -t 3 -q 100

reference: 
http://journal.code4lib.org/articles/4893
4.2. The isis2json conversion tool
To convert ISIS records into JSON structures, we developed a Python script called isis2json.py (BIREME, 2010b). It can be executed with both the Python and Jython interpreters, versions 2.5 through 2.7. When running under Python it can read only ISO-2709 files, but as a Jython script it leverages the Bruma Java library (Barbieri, 2011) and can also read binary ISIS files in .MST format directly. Several options control the structure of the JSON output. For example, the command line below generates output suitable for batch importing to CouchDB:

$ ./isis2json.py cds.iso -c -t 3 -q 100 > cds1.json
The arguments used in the above example are:

-c to generate records within a “docs” list, as required by the CouchDB _bulk_docs API;

-t 3 for ISIS-JSON type 3 export (fields as dictionaries of subfields);

-q 100 to output only 100 records;

Here is the help screen of isis2json.py:

$ ./isis2json.py -h
usage: isis2json.py [-h] [-o OUTPUT.json] [-c] [-m] [-t ISIS_JSON_TYPE]
                    [-q QTY] [-s SKIP] [-i TAG_NUMBER] [-u] [-p PREFIX] [-n]
                    [-k TAG:VALUE]
                    INPUT.(mst|iso)

Convert an ISIS .mst or .iso file to a JSON array

positional arguments:
  INPUT.(mst|iso)       .mst or .iso file to read

optional arguments:
  -h, --help            show this help message and exit
  -o OUTPUT.json, --out OUTPUT.json
                        the file where the JSON output should be written
                        (default: write to stdout)
  -c, --couch           output array within a "docs" item in a JSON document
                        for bulk insert to CouchDB via POST to db/_bulk_docs
  -m, --mongo           output individual records as separate JSON
                        dictionaries, one per line for bulk insert to MongoDB
                        via mongoimport utility
  -t ISIS_JSON_TYPE, --type ISIS_JSON_TYPE
                        ISIS-JSON type, sets field structure: 1=string,
                        2=alist, 3=dict
  -q QTY, --qty QTY     maximum quantity of records to read (default=ALL)
  -s SKIP, --skip SKIP  records to skip from start of .mst (default=0)
  -i TAG_NUMBER, --id TAG_NUMBER
                        generate an "_id" from the given unique TAG field
                        number for each record
  -u, --uuid            generate an "_id" with a random UUID for each record
  -p PREFIX, --prefix PREFIX
                        concatenate prefix to every numeric field tag (ex. 99
                        becomes "v99")
  -n, --mfn             generate an "_id" from the MFN of each record
                        (available only for .mst input)
  -k TAG:VALUE, --constant TAG:VALUE
                        Include a constant tag:value in every record (ex. -k
                        type:AS)
