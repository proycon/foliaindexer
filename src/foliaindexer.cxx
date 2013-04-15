
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <sys/stat.h>
#include <unordered_map>
#include "boost/filesystem.hpp"
#include "unicode/unistr.h"
#include "libfolia/document.h"
#include "libfolia/folia.h"

using namespace std;

enum OutputMode { TAB, CSV, SQLITE, MYSQL } outputmode = TAB;

unsigned int currentkey = 0;
string delimiter = "";
ostream * f_elements = (ostream *) &cout;
ostream * f_annotations = (ostream *) &cout;


void usage(){
  cerr << "usage: foliaindexer [--tab|--csv|--mysql] [<foliafile>..|<foliadir>..]*" << endl;
}


void maketypepath(folia::FoliaElement * e, string & path) {
    path = e->xmltag() + "/" + path;
    if (e->parent() != NULL) maketypepath(e->parent(),path);
}

bool isstructureannotation(folia::FoliaElement * e) {
    //TODO: move to libfolia
    return ((e->element_id() >= folia::Text_t) && (e->element_id() <= folia::Quote_t));
}

bool istokenannotation(folia::FoliaElement * e) {
    return ((e->element_id() >= folia::Pos_t) && (e->element_id() <= folia::Metric_t));
}


void preparedb() {
    if (outputmode == SQLITE) {
        *f_elements << "create table elements('key' int primary key, 'id' varchar(255), 'type' varchar(50), 'parentkey' int, 'parenttype' varchar(50), 'typepath' text, 'next' int, 'previous' int, 'set' varchar(255), 'cls' varchar(255), annotator varchar(255), annotatortype boolean);" << endl;
        *f_annotations << "create table annotations('key' int primary key, 'annotationkey' int);" << endl;
    }

}

string sqlwrapescape(string s) {
    if ((s.empty()) || (s == "NULL")) return "NULL";
    if ((s.find('\'') != string::npos)) {
        string s2 = "";
        for (int i = 0; i < s.size(); i++) {
            if (s[i] == '\'') s2 += "\\'"; else s2 += s[i];
        }
        return s2;
    }
    return "'" + s + "'";

}



void printelement(folia::FoliaElement * e, folia::FoliaElement * parent,  folia::FoliaElement * next, folia::FoliaElement * prev, unordered_map<size_t, unsigned int> & keys) {
    const unsigned int key = keys[(size_t) e];
    if (e->isinstance(folia::WordReference_t)) {

        folia::FoliaElement * word = (e->doc()->index(e->id()));
        unsigned int wordkey = keys[(size_t) word];

        *f_annotations << wordkey << "\t" << key << endl;


    } else {


            // key     id     type     parentkey   parenttype    typepath  next    previous    text   set     cls     annotator   annotatortype  datetime 
            string parenttype = "folia";
            unsigned int parentkey = 0;
            if (parent != NULL) {
                parenttype = parent->xmltag(); 
                parentkey = keys[(size_t) parent];
            }
            string typepath;
            maketypepath(parent, typepath);


            stringstream next_s;
            if (next != NULL) next_s << keys[(size_t) next]; else next_s << "NULL";

            stringstream prev_s;
            if (prev != NULL) prev_s << keys[(size_t) prev]; else prev_s << "NULL";

            UnicodeString text = "NULL";
            if (e->printable()) {
                try {
                    text = e->text();
                } catch (folia::NoSuchText &e) {};
            }
            stringstream text_ss;
            text_ss << text;
            string text_s = text_ss.str();

            string annotatortype; 
            if (e->annotatortype() == folia::AnnotatorType::MANUAL) {
                annotatortype = "0";
            } else {
                annotatortype = "1";
            }




        if (outputmode == TAB) {
            *f_elements << key << delimiter << e->id() << delimiter << e->xmltag() << delimiter << (int) parentkey << delimiter << parenttype << delimiter << typepath << delimiter <<  next_s.str() << delimiter <<  prev_s.str() << delimiter << text << delimiter << e->sett() << delimiter << e->cls() << delimiter << e->annotator() << delimiter << annotatortype << endl;
        } else if (outputmode == SQLITE) {
            *f_elements << "insert into elements values(" << key << "," << sqlwrapescape(e->id()) << "," << sqlwrapescape(e->xmltag()) <<"," << (int) parentkey << "," << sqlwrapescape(parenttype) <<  "," << sqlwrapescape(typepath) << ","  << next_s.str() <<  "," <<  prev_s.str() << "," <<  sqlwrapescape(text_s) << "," << sqlwrapescape(e->sett()) <<  "," << sqlwrapescape(e->cls()) <<  "," << sqlwrapescape(e->annotator()) <<  "," << annotatortype << ");" << endl;
        }

        if (isstructureannotation(parent) &&  istokenannotation(e)) {
            if (outputmode == TAB) {
                *f_annotations << parentkey << "\t" << key << endl;                
            } else if (outputmode == SQLITE) {
                *f_annotations << "insert into annotations values("<< parentkey << "," << key << ");" << endl;                
            }
        }
    }
}



void processelement(folia::FoliaElement * e, unordered_map<size_t, unsigned int> & keys, folia::FoliaElement * overrideparent = NULL) {

    //reserve IDs
    for (size_t i = 0; i < e->size(); i++) {
        folia::FoliaElement * e2 = (*e)[i];
        if (keys.count((size_t) e2) == 0) {
            currentkey++;
            keys[(size_t) e2] = currentkey;
        }
    }

    for (size_t i = 0; i < e->size(); i++) {
        folia::FoliaElement * e2 = (*e)[i];

        if (e2->isinstance(folia::Correction_t)) {
            e2 = e2->getNew();
            if (e2 != NULL) processelement(e2, keys, e);

        } else if (e2->isinstance(folia::AnnotationLayer_t)) {
            //layers themselves need not be included
            processelement(e2, keys, e);
                    
        } else if ((!e2->isinstance(folia::TextContent_t)) && (!e2->isinstance(folia::Alternative_t)) && (!e2->isinstance(folia::Alternatives_t)))  { 

            folia::FoliaElement * next = (i < e->size() - 1 ? (*e)[i+1] : NULL);
            folia::FoliaElement * prev = (i > 0) ? (*e)[i-1] : NULL;

            printelement(e2,(overrideparent != NULL) ? overrideparent : e, next, prev, keys);
            processelement(e2, keys);
            
        }
    }
}


void processfile(const string & file) {
    cerr << "Processing " << file << endl;
    folia::Document doc;
    doc.readFromFile(file);
    for (int i = 0; i < doc.size(); i++) {
        folia::FoliaElement * e = doc[i];
        unordered_map<size_t, unsigned int> keys;
        processelement(e, keys);
    }
}


int processdir(const string & dir) {
  cerr << "Processing directory " << dir << endl;
  int counter = 0;

  if ( !boost::filesystem::exists( dir ) ) return false;
  boost::filesystem::directory_iterator end_itr; // default construction yields past-the-end
  for ( boost::filesystem::directory_iterator itr( dir ); itr != end_itr; ++itr ) {
    if (  boost::filesystem::is_directory(itr->status()) ) {
      const string d = itr->path().string();
      counter += processdir( d );
    } else if (( itr->path().filename().string() != ".") && (itr->path().filename().string() != ".." )) {
      processfile( itr->path().string() );
      counter++;
    }
  }
  return counter;
}



int main( int argc, char* argv[] ){
  if ( argc < 2 ){
    usage();
    exit(2);
  }

  
  for (int i = 1; i < argc; i++) {
    const string f = argv[i];
    if (f == "--mysql") {
        outputmode = MYSQL;
    } else if (f == "--sqlite") {
        outputmode = SQLITE;
    } else if (f == "--csv") {
        outputmode = CSV;
    } else if (f == "--tab") {
        outputmode = TAB;
        delimiter = "\t";
    } else {
        preparedb();
        boost::filesystem::path fp(argv[i]);
        if (boost::filesystem::exists(fp)) {
            if (boost::filesystem::is_directory(fp))  {
                processdir(f);
            } else {
                processfile(f);
            }
        } else {
            cerr << "ERROR: File or directory " << f << " does not exist" << endl;
        }
    }
  }

  exit(0);
}
