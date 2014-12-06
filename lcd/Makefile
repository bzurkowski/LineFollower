OBJS = lcd.o lcd_test.o
CC = gcc
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = -Wall $(DEBUG)

p : $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o p

lcd.o : lcd.h lcd.cpp
	$(CC) $(CFLAGS) lcd.cpp
	
lcd_test.o : lcd_test.cpp lcd.h
	$(CC) $(CFLAGS) lcd_test.cpp
	
clean:
	\rm *.o *~p