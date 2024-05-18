
typedef struct SongMetadata SongMetadata, *PSongMetadata;

typedef enum VocalGender {
    kUnknown=0,
    kMale=1,
    kFemale=2
} VocalGender;

struct SongMetadata {
    int mID;
    short mVersion;
    char mGameOrigin[16];
    float mPreviewStartMs;
    float mPreviewEndMs;
    char mTitle[256];
    char mArtist[256];
    char mAlbum[256];
    short mAlbumTrackNum;
    int mAlbumYear;
    int mOriginalYear;
    char mGenre[64];
    float mLengthMs;
    float mGuitarRank;
    float mBassRank;
    float mVocalsRank;
    float mDrumRank;
    float mBandRank;
    float mKeysRank;
    float mRealKeysRank;
    bool mIsTutorial;
    bool mHasAlbumArt;
    bool mIsCoverRecording;
    enum VocalGender mVocalGender;
    char mDeprecatedAnimTempo[16];
    bool mHasMarkup;
    int mNumVocalParts;
    uint mSolos;
    bool mIsFake;
    char mShortName[256];
};

