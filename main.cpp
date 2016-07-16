//
// Created by Purushotham Swaminathan on 7/5/16.
//

// Schema Validator example

// The example validates JSON text from stdin with a JSON schema specified in the argument.

#include <rapidjson/error/en.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/schema.h>
#include <rapidjson/stringbuffer.h>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
using namespace rapidjson;
using namespace std;

vector<string> &split(const string &s, char delim, vector<string> &elems);
vector<string> split(const string &s, char delim);

vector<string> &split(const string &s, char delim, vector<string> &elems){
    stringstream ss(s);
    string item;
    while(getline(ss,item,delim)){
        elems.push_back(item);
    }
    return elems;
}

vector<string> split(const string &s, char delim){
    vector<string> elems;
    split(s,delim,elems);
    return elems;
}
Document getDocumentFromFile(string fileUri);
class Resolver : public IRemoteSchemaDocumentProvider{
public:
    virtual const SchemaDocument* GetRemoteDocument(const Ch* uri, SizeType length);
};

const SchemaDocument* Resolver::GetRemoteDocument(const Ch* uri, SizeType length){
    //printf("%s, %d",uri, length);
    length = length + 0;
    vector<string> split_stuff = split(uri, '#');
    //cout<<endl;

    if(split_stuff.size()!=2){
        cout<<"Something wrong."<<endl;
        exit(-1);
    }
    string fileUri = split_stuff[0];
    string jsonPointer = split_stuff[1];


    //cout<<endl<<fileUri<<" and "<<jsonPointer<<endl;

    Document d = getDocumentFromFile(fileUri);
    Resolver resolver;

    const SchemaDocument* schemaDocument;
    schemaDocument = new SchemaDocument(d, &resolver);
    return schemaDocument;
}

Document getDocumentFromFile(string fileUri){
    Document d;
    char buffer[4096];

    {
        FILE *fp = fopen(fileUri.c_str(), "r");
        if (!fp) {
            printf("Schema file '%s' not found\n", fileUri.c_str());
            exit(-1);
        }
        FileReadStream fs(fp, buffer, sizeof(buffer));
        d.ParseStream(fs);
        if (d.HasParseError()) {
            fprintf(stderr, "Schema file '%s' is not a valid JSON\n", fileUri.c_str());
            fprintf(stderr, "Error(offset %u): %s\n",
                    static_cast<unsigned>(d.GetErrorOffset()),
                    GetParseError_En(d.GetParseError()));
            fclose(fp);
            exit(-1);
        }
        fclose(fp);
    }
    return d;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: refresolver schema.json < input.json\n");
        return EXIT_FAILURE;
    }

    // Read a JSON schema from file into Document
    Document d = getDocumentFromFile(argv[1]);

    // Then convert the Document into SchemaDocument

    Resolver resolver;

    // Use reader to parse the JSON in stdin, and forward SAX events to validator
    SchemaDocument schemaDocument(d, &resolver);
    SchemaValidator validator(schemaDocument);

    char buffer[4096];
    Reader reader;
    FileReadStream is(stdin, buffer, sizeof(buffer));
    if (!reader.Parse(is, validator) && reader.GetParseErrorCode() != kParseErrorTermination) {
        // Schema validator error would cause kParseErrorTermination, which will handle it in next step.
        fprintf(stderr, "Input is not a valid JSON\n");
        fprintf(stderr, "Error(offset %u): %s\n",
                static_cast<unsigned>(reader.GetErrorOffset()),
                GetParseError_En(reader.GetParseErrorCode()));
    }

    // Check the validation result
    if (validator.IsValid()) {
        printf("Whatever that was in inputstream is valid.\n");
        return EXIT_SUCCESS;
    }
    else {
        printf("Input JSON is invalid.\n");
        StringBuffer sb;
        validator.GetInvalidSchemaPointer().Stringify(sb);
        fprintf(stderr, "Invalid schema: %s\n", sb.GetString());
        fprintf(stderr, "Invalid keyword: %s\n", validator.GetInvalidSchemaKeyword());
        sb.Clear();
        validator.GetInvalidDocumentPointer().StringifyUriFragment(sb);
        fprintf(stderr, "Invalid document: %s\n", sb.GetString());
        return EXIT_FAILURE;
    }
}
