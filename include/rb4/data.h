
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

typedef struct _DataNodes
{
    DataNode n[1];
} DataNodes;

typedef struct _DataArray {
    DataNodes* mNodes;
    Symbol mFile;
    int mRefs;
    short mSize;
    ushort mLine;
} DataArray;

typedef DataNode* DataFunc(DataNode*, DataArray*);

typedef unsigned char   undefined;

typedef struct _SystemOptions {
        char pad[136];
        float mOverscan;
        float mAudioOffset;
        float mVideoOffset;
        float mDialogVolume;
        float mGammaValue;
        bool mHasLoaded;
        bool mHasCalibrated;
        undefined field55_0x46;
        undefined field56_0x47;
        char mStore[0x68];
} SystemOptions;

extern float (*DataNodeFloat)(DataNode*, DataArray*);
extern int (*DataNodeInt)(DataNode*, DataArray*);
extern DataNode* (*DataExecuteString)(DataNode* __return_storage_ptr__, char* param_1);
extern Symbol(*DataNodeForceSym)(DataNode*, DataArray*);
extern Symbol(*Symbol_Ctor)(Symbol*, const char*);

