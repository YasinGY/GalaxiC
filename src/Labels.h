#include "PCH.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch"
class Label{
public:
    inline Label(){
        m_LastLabel = LabelType::None;
        m_CurrentLabel = LabelType::main;

        m_MainLabels = 0;
        m_TempLabels = 0;
        m_BoolLabels = 0;
        m_IfLabels = 0;
        m_ChainLabels = 0;
        m_LoopLabels = 0;
    }

    enum class LabelType{
        None = 0,
        main, temp, _if, _bool, chain, loop
    };

    inline void addMainLabel() { m_MainLabels++; }
    inline void addTempLabel() { m_TempLabels++; }
    inline void addBoolLabel() { m_BoolLabels++; }
    inline void addIfLabel() { m_IfLabels++; }
    inline void addChainLabel() { m_ChainLabels++; }
    inline void addLoopLabel() { m_LoopLabels++; }

    inline void addLastLabel() {
        switch(m_LastLabel){
            case LabelType::main:
                addMainLabel();
                return;
            case LabelType::temp:
                addBoolLabel();
                return;
            case LabelType::_bool:
                addBoolLabel();
                return;
            case LabelType::_if:
                addIfLabel();
                return;
            case LabelType::chain:
                addChainLabel();
                return;
            case LabelType::loop:
                addLoopLabel();
                return;
            default:
                Log::Error("How has this happened?");
                exit(1);
        }
    }
    inline void addCurrentLabel() {
        switch(m_CurrentLabel){
            case LabelType::main:
                addMainLabel();
                return;
            case LabelType::temp:
                addBoolLabel();
                return;
            case LabelType::_bool:
                addBoolLabel();
                return;
            case LabelType::_if:
                addIfLabel();
                return;
            case LabelType::chain:
                addChainLabel();
                return;
            case LabelType::loop:
                addLoopLabel();
                return;
            default:
                Log::Error("How has this happened?");
                exit(1);
        }
    }

    inline std::string getMainLabel(const bool declared_label = true) {
        if(declared_label) {
            m_LastLabel = m_CurrentLabel;
            m_CurrentLabel = LabelType::main;
        }
        return "main" + std::to_string(m_MainLabels);
    }
    inline std::string getTempLabel(const bool declared_label = true) {
        if(declared_label) {
            m_LastLabel = m_CurrentLabel;
            m_CurrentLabel = LabelType::temp;
        }
        return "temp" + std::to_string(m_TempLabels);
    }
    inline std::string getBoolLabel(const bool declared_label = true) {
        if(declared_label) {
            m_LastLabel = m_CurrentLabel;
            m_CurrentLabel = LabelType::_bool;
        }
        return "bool" + std::to_string(m_BoolLabels);
    }
    inline std::string getIfLabel(const bool declared_label = true) {
        if(declared_label) {
            m_LastLabel = m_CurrentLabel;
            m_CurrentLabel = LabelType::_if;
        }
        return "if" + std::to_string(m_IfLabels);
    }
    inline std::string getChainLabel(const bool declared_label = true) {
        if(declared_label) {
            m_LastLabel = m_CurrentLabel;
            m_CurrentLabel = LabelType::chain;
        }
        return "chain" + std::to_string(m_ChainLabels);
    }
    inline std::string getLoopLabel(const bool declared_label = true){
        if(declared_label) {
            m_LastLabel = m_CurrentLabel;
            m_CurrentLabel = LabelType::loop;
        }
        return "loop" + std::to_string(m_LoopLabels);
    }

    inline std::string getLastLabel(const bool declared_label = true){
        switch(m_LastLabel){
            case LabelType::main:
                return getMainLabel(declared_label);
            case LabelType::temp:
                return getTempLabel(declared_label);
            case LabelType::_if:
                return getIfLabel(declared_label);
            case LabelType::_bool:
                return getBoolLabel(declared_label);
            case LabelType::chain:
                return getChainLabel(declared_label);
            case LabelType::loop:
                return getLoopLabel(declared_label);
        }

        return "";
    }

    inline std::string getCurrentLabel(const bool declared_label = true){
        switch(m_CurrentLabel){
            case LabelType::main:
                return getMainLabel(declared_label);
            case LabelType::temp:
                return getTempLabel(declared_label);
            case LabelType::_if:
                return getIfLabel(declared_label);
            case LabelType::_bool:
                return getBoolLabel(declared_label);
            case LabelType::chain:
                return getChainLabel(declared_label);
            case LabelType::loop:
                return getLoopLabel(declared_label);
        }

        return "";
    }

    uint64_t getLabelAmount(const LabelType type) {
        switch(type){
            case LabelType::main:
                return m_MainLabels;
            case LabelType::temp:
                return m_TempLabels;
            case LabelType::_bool:
                return m_BoolLabels;
            case LabelType::_if:
                return m_IfLabels;
            case LabelType::chain:
                return m_ChainLabels;
            case LabelType::loop:
                return m_LoopLabels;
        }
        return -1;
    }
    void setLabelAsCurrent(const LabelType type) {
        switch(type){
            case LabelType::main:
                m_LastLabel = m_CurrentLabel;
                m_CurrentLabel = LabelType::main;

            case LabelType::temp:
                m_LastLabel = m_CurrentLabel;
                m_CurrentLabel = LabelType::temp;

            case LabelType::_bool:
                m_LastLabel = m_CurrentLabel;
                m_CurrentLabel = LabelType::_bool;

            case LabelType::_if:
                m_LastLabel = m_CurrentLabel;
                m_CurrentLabel = LabelType::_if;

            case LabelType::chain:
                m_LastLabel = m_CurrentLabel;
                m_CurrentLabel = LabelType::chain;

            case LabelType::loop:
                m_LastLabel = m_CurrentLabel;
                m_CurrentLabel = LabelType::loop;
        }
    }

private:

    LabelType m_LastLabel;
    LabelType m_CurrentLabel;
    uint64_t m_MainLabels;
    uint64_t m_TempLabels;
    uint64_t m_IfLabels;
    uint64_t m_BoolLabels;
    uint64_t m_ChainLabels;
    uint64_t m_LoopLabels;
};
#pragma clang diagnostic pop