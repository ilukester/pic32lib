#ifndef PROPERTIES_H
#define PROPERTIES_H

#include "Core.h"
#include "KeyValueTable.h"
#include "TokenParser.h"
#include "StringList.h"

class Properties : public KeyValueTable {
public:
    typedef void (*fptr_string)(String);

    typedef enum {
        NullProperty = 0,
        BoolProperty,
        NumberProperty,
        StringProperty,
        JsonProperty
    } PropertyType;

    typedef enum {
        RO = 0,
        RW
    } PropertyMode;

    typedef struct {
        PropertyType type;
        PropertyMode mode;
        fptr_string changeHandler;
    } PropertiesTableDetail;

    Properties(us8 size = 16)
    {
        propertiesTable = (PropertiesTableDetail*)malloc(size * sizeof(PropertiesTableDetail));

        for(us8 i = 0; i < tableSize; i++) {
            propertiesTable[i].type = NullProperty;
            propertiesTable[i].mode = RW;
            propertiesTable[i].changeHandler = 0;
        }

        echoFunction = 0;
    }

    s8 addBool(String key, bool value, fptr_string func = 0, PropertyMode mode = RW)
    {
        s8 index = setValue(key, value ? "true" : "false", true);
        if(index >= 0) {
            propertiesTable[index].type = BoolProperty;
            propertiesTable[index].mode = mode;
            if(func != 0) {
                propertiesTable[index].changeHandler = func;
            }
        }
    }

    s8 addNumber(String key, String value, fptr_string func = 0, PropertyMode mode = RW)
    {
        s8 index = setValue(key, value, true);
        if(index >= 0) {
            propertiesTable[index].type = NumberProperty;
            propertiesTable[index].mode = mode;
            if(func != 0) {
                propertiesTable[index].changeHandler = func;
            }
        }
    }

    s8 addString(String key, String value, fptr_string func = 0, PropertyMode mode = RW)
    {
        s8 index = setValue(key, value, true);
        if(index >= 0) {
            propertiesTable[index].type = StringProperty;
            propertiesTable[index].mode = mode;
            if(func != 0) {
                propertiesTable[index].changeHandler = func;
            }
        }
    }

    s8 addJson(String key, String value, fptr_string func = 0, PropertyMode mode = RW)
    {
        s8 index = setValue(key, value, true);
        if(index >= 0) {
            propertiesTable[index].type = JsonProperty;
            propertiesTable[index].mode = mode;
            if(func != 0) {
                propertiesTable[index].changeHandler = func;
            }
        }
    }

    bool update(String key, String value)
    {
        s8 index = findIndex(key);
        return update(index, value);
    }

    bool update(s8 index, String value)
    {
        if(0 <= index && index < tableSize) {
            setValue(index, value);

            if(echoFunction != 0) {
                echoFunction(jsonString(index));
            }

            if(propertiesTable[index].changeHandler != 0) {
                propertiesTable[index].changeHandler(value);
            }
            return true;
        }
        return false;
    }

    String jsonString(s8 index)
    {
        if(0 <= index && index < tableSize) {
            StringList list;
            list << key(index);
            list << value(index);

            switch(propertiesTable[index].type) {
            case NullProperty:
                return list.augment("{ \"%1\": null }");

            case BoolProperty:
            case NumberProperty:
            case JsonProperty:
                return list.augment("{ \"%1\": %2 }");

            case StringProperty:
                return list.augment("{ \"%1\": \"%2\" }");
            }
        }
        return "";
    }

    void evaluate(TokenParser &parser, bool ignoreJson = 0)
    {
        command(parser);

        if(parser.isJSON() || ignoreJson) {
            String key = parser.toString();

            s8 index = findIndex(key);
            if(0 <= index && index < tableSize) {
                if(propertiesTable[index].mode == RW) {
                    parser.nextToken();
                    String value = parser.toString();

                    setValue(index, value);

                    if(propertiesTable[index].changeHandler != 0) {
                        propertiesTable[index].changeHandler(value);
                    }
                }
                else {
                    if(echoFunction != 0) {
                        StringList list;
                        list << key;

                        echoFunction(list.augment("{ \"error\": \"%1 is read only\" }"));
                    }
                }
            }
        }
    }

    void setEchoFunction(fptr_string func)
    {
        echoFunction = func;
    }

    void command(TokenParser &parser)
    {
        if(parser.compare("get")) {
            parser.nextToken();
            s8 index = findIndex(parser.toString());
            parser.println(jsonString(index));
        }
        else if(parser.compare("set")) {
            parser.nextToken();
            s8 index = findIndex(parser.toString());
            parser.nextToken();
            update(index, parser.toString());
        }
        else if(parser.compare("keys")) {
            us8 count = 0;
            parser.print("[");
            for(s8 i = 0; i < tableSize; i++) {
                if(kvTable[i].key[0] != 0) {
                    if(count++ > 0) {
                        parser.print(",");
                    }
                    parser.print("\"");
                    String key(kvTable[i].key);
                    parser.print(key);
                    parser.print("\"");
                }
            }
            parser.println("]");
        }
        else if(parser.compare("properties")) {
            us8 length;
            for(length = 0; length < tableSize; length++) {
                if(kvTable[length].key[0] == 0) {
                    break;
                }
            }

            parser.print("{\n");
            for(us8 i = 0; i < length; i++) {
                StringList list;
                list << key(i);
                list << String((propertiesTable[i].mode == RW) ? "rw" : "ro");

                switch(propertiesTable[i].type) {
                case NullProperty:
                    list << "null";
                    list << "null";
                    break;
                case BoolProperty:
                    list << "bool";
                    list << String((value(i) == "true") ? "true" : "false");
                    break;
                case NumberProperty:
                    list << "number";
                    list << value(i);
                    break;
                case StringProperty:
                    list << "string";
                    list << String("\"" + value(i) + "\"");
                    break;
                case JsonProperty:
                    list << "json";
                    list << value(i);
                }

                parser.print(list.augment("\t\"%1\": [\"%2\", \"%3\", %4]"));
                if(i < (length - 1)) {
                    parser.print(",");
                }
                parser.print("\n");
            }
            parser.print("}\r");
        }
    }

private:
    PropertiesTableDetail *propertiesTable;
    fptr_string echoFunction;
};

#endif // PROPERTIES_H