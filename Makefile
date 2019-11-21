all:
	gcc cnf/sha512.c cnf/mphf-cnf2.c -O2 -o cnf/encode
	gcc decode/sha512.c decode/decode.c -O2 -o decode/decode
