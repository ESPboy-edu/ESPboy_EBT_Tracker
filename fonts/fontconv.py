from image256c import *
import sys



picture_src=Image()
picture_src.load(sys.argv[1])

name=os.path.splitext(sys.argv[1])[0]

print('const uint8_t '+name+'[] PROGMEM ={')

char_w=int(picture_src.wdt/16)
char_h=int(picture_src.hgt/8)

for i in range(0,8):

	for j in range(0,16):
	
		str=''
		
		for k in range(0,char_h):
			line=0
			bitc=0
			for l in range(0,char_w):
				if bitc==8:
					str+='0x%2.2x,' % line
					line=0
					bitc=0
				if picture_src.get(j*char_w+l,i*char_h+k)!=0:
					line=line|(1<<(l&7))
				bitc+=1

			str+='0x%2.2x,' % line
			
		print(str)

print('};')
