import os
from gne import GeneralNewsExtractor

filename = []
fp = open("/home/ianliu/develope/maintex_Extraction/mytest.txt", "r")
lines = fp.readlines()

for line in lines:
    line = line.split("\n")[0]
    filename.append(line)


extractor = GeneralNewsExtractor()
for f in filename:
    html = open(f, "r")
    print(f)
    result = extractor.extract(html.read())
    '''print(f.split(".")[0].split("/") )'''
    wf = open("/home/ianliu/develope/maintex_Extraction/python/output/" +  f.split(".")[0].split("/")[8]  + ".txt", "w")
    wf.write(result['content'])
    wf.close()
    html.close()
