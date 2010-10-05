all: server 

server: 
	$($(shell generate_server -Dservice.names=CREDIT,DEBIT -Dserver.includes=CreditService.c,DebitService.c))

clean:
	rm -f server 
