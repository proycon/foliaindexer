
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

enum OutputMode { TAB, CSV, MYSQL } outputmode = TAB;

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


void printelement(folia::FoliaElement * e, folia::FoliaElement * parent, unsigned int parentkey, folia::FoliaElement * next, folia::FoliaElement * prev, unordered_map<size_t, unsigned int> & keys) {
    const unsigned int key = keys[(size_t) e];
    if (e->isinstance(folia::WordReference_t)) {

        folia::FoliaElement * word = (e->doc()->index(e->id()));
        unsigned int wordkey = keys[(size_t) word];

        *f_annotations << wordkey << "\t" << key << endl;


    } else {


        if (outputmode == TAB) {
            // key     id     type     parentkey   parenttype    typepath  next    previous    text   set     cls     annotator   annotatortype  datetime 
            string parenttype = "folia";
            unsigned int parentkey = 0;
            if (parent != NULL) {
                parenttype = parent->xmltag(); 
                parentkey = keys[(size_t) parent];
            }
            string typepath;
            maketypepath(parent, typepath);


            string next_s = "NULL";
            if (next != NULL) next_s = keys[(size_t) next];

            string prev_s = "NULL";
            if (prev != NULL) prev_s = keys[(size_t) prev];

            string text = "NULL";
            if (e->printable()) text = e->text();




            *f_elements << key << delimiter << e->id() << delimiter << e->xmltag() << delimiter << parentkey << delimiter << parenttype << delimiter << typepath << delimiter << next_s << delimiter << prev_s << delimiter << text <<  e->set() << delimiter << e->cls() << delimiter << e->annotator() << delimiter << e->annotatortype() << endl;

            if (isstructureannotation(parent) &&  istokenannotation(e)) {
                *f_annotations << parentkey << "\t" << key << endl;                
            }


        }
    }
}



void processelement(folia::FoliaElement * e, unordered_map<size_t, unsigned int> & keys, folia::FoliaElement * overrideparent = NULL) {

    //reserve IDs
    for (int i = 0; i < e->size(); i++) {
        folia::FoliaElement * e2 = (*e)[i];
        if (keys.count((size_t) e2) == 0) {
            currentkey++;
            keys[(size_t) e2] = currentkey;
        }
    }

    for (int i = 0; i < e->size(); i++) {
        folia::FoliaElement * e2 = (*e)[i];

        if (e2->isinstance(folia::Correction_t)) {
            e2 = e2->getNew();
            if (e2 != NULL) processelement(e2, keys);

        } else if (e2->isinstance(folia::AnnotationLayer_t)) {
            //layers themselves need not be included
            processelement(e2, keys);
                    
        } else if ((!e2->isinstance(folia::TextContent_t)) && (!e2->isinstance(folia::Alternative_t)) && (!e2->isinstance(folia::Alternatives_t)))  { 

            folia::FoliaElement * next = (i < size() - 1 ? (*e)[i+1] : NULL);
            folia::FoliaElement * prev = (i > 0) ? (*e)[i-1] : NULL;

            printelement(e2, key, (overrideparent != NULL) ? overrideparent : e, next, prev, keys);
            processelement(e2)
            
        }
    }
}


void processfile(const string & file) {
    cerr << "Processing " << file << endl;
    folia::Document doc;
    doc.readFromFile(file);
    for (int i = 0; i < doc.size(); i++) {
        folia::FoliaElement e = doc[i];
        processelement(e);
        
    }
}


int processdir(const string & dir) {
  cerr << "Processing directory " << dir << endl;
  counter = 0;

  if ( !boost::filesystem::exists( dir ) ) return false;
  boost::filesystem::exists directory_iterator end_itr; // default construction yields past-the-end
  for ( boost::filesystem::directory_iterator itr( dir_path ); itr != end_itr; ++itr ) {
    if (  boost::filesystem::is_directory(itr->status()) ) {
      counter += processdir( itr->path() );
    } else if (( itr->leaf() != ".") && (itr->lead() != ".." )) {
      processfile( itr->path() )
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
    } else if (f == "--csv") {
        outputmode = CSV;
    } else if (f == "--tab") {
        outputmode = TAB;
        delimiter = "\t"
    } else {
        boost::filesystem::path fp(argv[i]);
        if (boost::filesystem::exists(fp))
            if (boost::filesystem::is_directory(fp))  {
                processdir(f);
            } else {
                processfile(f);
            }
        } else {
            cerr << "ERROR: File or directory " << f << " does not exist" << endl;
        }
  }

  exit(0);
}
