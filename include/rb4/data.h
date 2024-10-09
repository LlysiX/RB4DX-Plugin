
typedef struct _Symbol {
    char* sym;
} Symbol;

typedef enum _DataType {
    kDataInt = 0,
    kDataFloat = 1,
    kDataVar = 2,
    kDataFunc = 3,
    kDataSink = 4,
    kDataSymbol = 5,
    kDataInvalid = 6,
    kDataIfdef = 7,
    kDataElse = 8,
    kDataEndif = 9,

    kDataArray = 16,
    kDataCommand = 17,
    kDataString = 18,
    kDataProperty = 19,
    kDataGlob = 20,
    kDataColor = 21,
    kDataPropRef = 22,

    kDataDefine = 32,
    kDataInclude = 33,
    kDataMerge = 34,
    kDataIfndef = 35,
    kDataAutorun = 36,
    kDataUndef = 37,

    kDataWaveformFloat = 38,
    kDataWaveformColor = 39,
    kDataObjectId = 40,
    kDataResourcePath = 41,
} DataType;

typedef union _DataNode_Value {
    char* symbol;
    float real;
    struct DataArray* array;
    struct DataNode(*func)(struct DataArray*);
    struct MsgSink* sink;
    // WaveformResource<float>* waveformFloat;
    // WaveformResource<Hmx::Color>* waveformColor;
    size_t value;
} DataNode_Value;

typedef struct _DataNode {
    DataNode_Value mValue;
    DataType mType;
} DataNode;

typedef struct _DataArray {
    DataNode* mNodes;
    Symbol mFile;
    int mRefs;
    short mSize;
    ushort mLine;
} DataArray;

typedef DataNode *DataFunc(DataNode*, DataArray*);