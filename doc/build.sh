#!/bin/bash
pandoc -s -S -c doc/github-pandoc.css  ./README -o index.html
