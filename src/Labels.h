#include "PCH.h"

class Label{
public:
    enum class LabelTypes{
        _main, _bool, _if, _loop
    };

    inline Label(){
        m_LastLabels.emplace_back(LabelTypes::_main);
    }

    inline LabelTypes GetCurrentLabelType(){ return m_LastLabels.at(m_LastLabels.size() - 1); }
    inline LabelTypes GetLastLabelType(int labels_ago = 1){ return m_LastLabels.at(m_LastLabels.size() - labels_ago - 1); }

    inline std::string GetMainLabel(){ return "main" + std::to_string(m_MainLabels); }
    inline std::string GetBoolLabel(){ return "bool" + std::to_string(m_BoolLabels); }
    inline std::string GetIfLabel(){ return "if" + std::to_string(m_IfLabels); }
    inline std::string GetLoopLabel(){ return "loop" + std::to_string(m_LoopLabels); }
    inline std::string GetLabelStringByType(LabelTypes type){
        switch(type){
            case LabelTypes::_main:
                return GetMainLabel();
            case LabelTypes::_bool:
                return GetBoolLabel();
            case LabelTypes::_if:
                return GetIfLabel();
            case LabelTypes::_loop:
                return GetLoopLabel();
        }
    }
    inline std::string GetCurrentLabel(){
        return GetLabelStringByType(GetCurrentLabelType());
    }

    inline void AddLabel(LabelTypes type, bool define_label = false){
        if(define_label && m_LastLabels.at(m_LastLabels.size() - 1) != type){
            m_LastLabels.emplace_back(type);
        }

        switch(type){
            case LabelTypes::_main:
                m_MainLabels++;
                break;
            case LabelTypes::_bool:
                m_BoolLabels++;
                break;
            case LabelTypes::_if:
                m_IfLabels++;
                break;
            case LabelTypes::_loop:
                m_LoopLabels++;
                break;
        }
    }

    inline void EndLabel(){
        m_LastLabels.pop_back();
    }

private:

    uint64_t m_MainLabels = 0;
    uint64_t m_BoolLabels = 0;
    uint64_t m_IfLabels = 0;
    uint64_t m_LoopLabels = 0;

    std::vector<LabelTypes> m_LastLabels;
};
