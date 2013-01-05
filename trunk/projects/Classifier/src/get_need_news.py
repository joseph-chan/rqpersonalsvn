#!/usr/bin/env python
#coding = gbk
import sys
import os
import re

content_re="@B:(.*?)\n@"
url_re="@U:(.*?)\n@"

target_url="http://eladies.sina.com.cn/qg"

if __name__ == "__main__":
	if len(sys.argv) != 3:
		print "./program_name input_dir output_file"
		sys.exit()
	output_file=open(sys.argv[2],'w')
	dirs=os.walk(sys.argv[1])
	print sys.argv[1]
	while True:
		try:
			this_dir=dirs.next()
		except:
			print "walk out!"
			break
		for one_file in this_dir[2]:
			file_content=open(this_dir[0]+"/"+one_file).read()
			url=re.findall(url_re,file_content)
			if len(url) !=1 :
				print "can't find url, continue, %s" % file_content
				continue
			url=url[0]
			if url.find(target_url) == -1:
				print "url not need : %s" % url
				continue

			content=re.findall(content_re,file_content)
			if len(content) !=1 or len(content[0])<10:
				print "content may be error quit, %s" % content
				continue

			content=content[0]
			content = "".join(re.split("<.*?>",content))
			output_file.write(content + "\n")
			
			




