
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

typedef DataNode* DataFunc(DataNode*, DataArray*);

typedef unsigned char   undefined;

typedef struct _SystemOptions {
        undefined field0_0x0;
        undefined field1_0x1;
        undefined field2_0x2;
        undefined field3_0x3;
        undefined field4_0x4;
        undefined field5_0x5;
        undefined field6_0x6;
        undefined field7_0x7;
        undefined field8_0x8;
        undefined field9_0x9;
        undefined field10_0xa;
        undefined field11_0xb;
        undefined field12_0xc;
        undefined field13_0xd;
        undefined field14_0xe;
        undefined field15_0xf;
        undefined field16_0x10;
        undefined field17_0x11;
        undefined field18_0x12;
        undefined field19_0x13;
        undefined field20_0x14;
        undefined field21_0x15;
        undefined field22_0x16;
        undefined field23_0x17;
        undefined field24_0x18;
        undefined field25_0x19;
        undefined field26_0x1a;
        undefined field27_0x1b;
        undefined field28_0x1c;
        undefined field29_0x1d;
        undefined field30_0x1e;
        undefined field31_0x1f;
        undefined field32_0x20;
        undefined field33_0x21;
        undefined field34_0x22;
        undefined field35_0x23;
        undefined field36_0x24;
        undefined field37_0x25;
        undefined field38_0x26;
        undefined field39_0x27;
        undefined field40_0x28;
        undefined field41_0x29;
        undefined field42_0x2a;
        undefined field43_0x2b;
        undefined field44_0x2c;
        undefined field45_0x2d;
        undefined field46_0x2e;
        undefined field47_0x2f;
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
