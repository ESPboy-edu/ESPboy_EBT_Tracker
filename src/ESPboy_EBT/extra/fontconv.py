from image256c import *
import sys



picture_src=Image()
picture_src.load(sys.argv[1])

name=os.path.splitext(sys.argv[1])[0]

print('const uint8_t '+name+'[] PROGMEM ={')

for i in range(0,8):

	for j in range(0,16):
	
		str=''
		
		for k in range(0,8):
			line=0
			for l in range(0,8):
				if picture_src.get(j*8+l,i*8+k)!=0:
					line=line|(1<<l)
			str+='0x%2.2x,' % line
			
		print(str)

print('};')
