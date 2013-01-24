#ifndef TOKENPARSER_H
#define TOKENPARSER_H

#include <WProgram.h>
#include <Stream.h>
#include "Core.h"
#include "Variant.h"

class TokenParser {
public:
    TokenParser(Stream* stream, us8 bufferSize = 64)
    {
        myStream = stream;
        buffer = (us8*)malloc(bufferSize + 1);
        reset();
        save();
        length = 0;
        stopCharactorFound = false;
    }

    bool scan(us8 stopCharactor = '\r')
    {
        if(stopCharactorFound) {
            stopCharactorFound = false;
            length = 0;
        }

        if(myStream->available() > 0) {
            us8 data = myStream->read();
            buffer[length++] = data;

            if(data == stopCharactor) {
                stopCharactorFound = true;
                buffer[length - 1] = ' ';
                reset();
                save();
                nextToken();
            }
        }
        return stopCharactorFound;
    }

    us8 getLength()
    {
        return length;
    }

    us8 getTail()
    {
        return tail;
    }

    us8 getHead()
    {
        return head;
    }

    us8 remaining()
    {
        return length - head;
    }

    bool isJSON()
    {
        us8 first = buffer[0];
        us8 last = buffer[length - 2];
        if((first == '{' && last == '}') || (first == '[' && last == ']')) {
            return true;
        }
        return false;
    }

    void reset()
    {
        head = 0;
        tail = 0;
    }

    void save()
    {
        savedHead = head;
        savedTail = tail;
    }

    void restore()
    {
        head = savedHead;
        tail = savedTail;
    }

    // todo: add bounds checking
    bool startsWith(const char* string, bool caseSensitive = false)
    {
      us8 i = 0;
      us8 index = tail;
      while(string[i] != 0) {
        if(string[i] != '?') {
            if(!characterCompare(buffer[index], string[i], caseSensitive)) {
                return false;
            }
        }
        i++;
        index++;
      }
      return true;
    }

    // Compares an entire string for equality
    bool compare(const char* string, bool caseSensitive = false)
    {
      us8 i = 0;
      us8 index = tail;
      while(string[i] != 0) {
        if(string[i] != '?') {
            if(!characterCompare(buffer[index], string[i], caseSensitive)) {
                return false;
            }
        }
        i++;
        index++;
      }

      if((head - tail) != i) {
          return false;
      }
      return true;
    }

    // todo: enforce consecutive charactors
    bool contains(const char* string, bool caseSensitive = false)
    {
        us8 i = 0;
        us8 index = tail;
        while(string[i] != 0 && index < length) {
            if(characterCompare(buffer[index++], string[i], caseSensitive)) {
                i++;
                if(string[i] == 0) {
                    return true;
                }
            }
        }
        return false;
    }

    String toString()
    {
        String temp;
        for(us8 i = tail; i < head; i++) {
            temp += buffer[i];
        }
        return temp;
    }

    bool nextToken()
    {
        tail = head;
        tail = skip(tail, true);

        head = tail;
        head = skip(head, false);

        return (tail < head) ? true : false;
    }

    bool advanceTail(us8 advance)
    {
        if(advance < (head - tail)) {
            tail += advance;
            return true;
        }
        return false;
    }

    bool reverseHead(us8 reverse)
    {
        if(reverse < (head - tail)) {
            head = head - reverse;
            return true;
        }
        return false;
    }

    void advanceHead(us8 advance)
    {
        us8 limit = length - head;
        if(advance < limit) {
            head += advance;
        }
        else {
            head += (limit - 1);
        }
    }

    us8 hexCharToNibble(us8 c) {
        // make upper case
        if('a' <= c && c <= 'z') {
            c -= 0x20;
        }

        if('0' <= c && c <= '9') {
            c -= 0x30;
        }
        else if('A' <= c && c <= 'F') {
            c -= 0x37;
        }
        return c & 0xf;
    }

    Variant toVariant()
    {
        return Variant(toString());
    }

    void print(const String &string)
    {
        myStream->print(string);
    }

    void println(const String &string)
    {
        myStream->println(string);
    }

private:
    inline bool characterCompare(us8 char1, us8 char2, bool caseSensitive = true)
    {
        if(char1 == char2) {
            return true;
        }

        if(!caseSensitive) {
            if(uppercase(char1) == uppercase(char2)) {
                return true;
            }
        }
        return false;
    }

    inline us8 uppercase(us8 x)
    {
        // make upper case
        if ('a' <= x && x <= 'z') {
            x -= 0x20;
        }
        return x;
    }

    us8 skip(us8 index, bool skipSeparator)
    {
        bool prevState = (index == 0) ? skipSeparator : isSeperator(index);

        while(0 <= index && index <= length) {
            bool currentstate = isSeperator(index);

            if(skipSeparator) {
                // falling edge
                if(prevState && !currentstate) {
                    break;
                }
            }
            else {
                // rising edge
                if(!prevState && currentstate) {
                    break;
                }
            }
            prevState = currentstate;
            index++;
        }

        if(index > length) {
            return length;
        }
        return index;
    }

    inline bool isSeperator(us8 index)
    {

        us8 tempChar = buffer[index];
        us8 array[] = {' ', '\n', ':', ',', '{', '}', '[', ']', '"'};

        for(us8 i = 0; i < sizeof(array); i++) {
            if(tempChar == array[i]) {
                return true;
            }
        }
        return false;
    }

    Stream* myStream;
    us8* buffer;
    us8 length;
    us8 head;
    us8 tail;
    us8 savedHead;
    us8 savedTail;
    bool stopCharactorFound;
};

#endif // TOKENPARSER_H