#include "InvertIndex.h"

namespace Skilo {
namespace Index{

InvertIndex::InvertIndex()
{

}

void InvertIndex::add_record(const IndexRecord &record)
{
    for(size_t i=0;i<record.terms.size();i++){
        string_view term=record.terms[i];
        TermEntry *term_entry=_index.find(term.data(),term.length());
        if(!term_entry){
            term_entry=new TermEntry();
            _index.insert(term.data(),term.length(),term_entry);
        }
        term_entry->doc_freq++;
        term_entry->posting_list.add_doc(record.seq_id);
    }
}


} //namespace Index
} //namespace Skilo
