class DotSep {
public:
    DotSep(const std::string& input) {
        size_t dotPosition = input.find('.');
        if (dotPosition != std::string::npos) {
            beforeDot = input.substr(0, dotPosition);
            afterDot = input.substr(dotPosition + 1);
            _hasDot = true;
        } else {
            beforeDot = input;
            afterDot = "";
        }
    }

    std::string getBeforeDot() const {
        return beforeDot;
    }

    std::string getAfterDot() const {
        return afterDot;
    }

    bool hasDot() {
        return _hasDot;
    }

private:
    std::string beforeDot;
    std::string afterDot;
    bool _hasDot;
};