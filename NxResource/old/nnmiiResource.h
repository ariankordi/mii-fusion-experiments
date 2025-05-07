// Generic

typedef struct Float3 {
    // Original is a union.
    float x;
    float y;
    float z;
} Float3;

// mii_DrawCommon.h

typedef struct BoundingBox {
    Float3 min;
    Float3 max;
} BoundingBox;

// mii_DrawParam.h

typedef enum AttributeType {
    AttributeType_Position=0, // AttributeFormat_16_16_16_16_Float / _p0
    AttributeType_Normal=1,   // AttributeFormat_10_10_10_2_Snorm / _n0
    AttributeType_Uv=2,       // AttributeFormat_16_16_Float / _u0
    AttributeType_Tangent=3,  // AttributeFormat_8_8_8_8_Snorm / _t0
    AttributeType_Param=4,    // AttributeFormat_8_8_8_8_Unorm / _c0 (Color)
    AttributeType_End=5
} AttributeType;

// mii_ResourceCommonHeader.h

typedef enum ResourceMemoryLevel {
    ResourceMemoryLevel_1=1,
    ResourceMemoryLevel_Min=1,
    ResourceMemoryLevel_2=2,
    ResourceMemoryLevel_3=3,
    ResourceMemoryLevel_4=4,
    ResourceMemoryLevel_5=5,
    ResourceMemoryLevel_6=6,
    ResourceMemoryLevel_7=7,
    ResourceMemoryLevel_8=8,
    ResourceMemoryLevel_Default=8,
    ResourceMemoryLevel_9=9,
    ResourceMemoryLevel_Max=9
} ResourceMemoryLevel;

typedef enum ResourceCompressLevel {
    ResourceCompressLevel_0=0,   // Z_NO_COMPRESSION
    ResourceCompressLevel_Min=0,
    ResourceCompressLevel_1=1,   // Z_BEST_SPEED
    ResourceCompressLevel_2=2,
    ResourceCompressLevel_3=3,
    ResourceCompressLevel_4=4,
    ResourceCompressLevel_5=5,
    ResourceCompressLevel_6=6,
    ResourceCompressLevel_Default=6,
    ResourceCompressLevel_7=7,
    ResourceCompressLevel_8=8,
    ResourceCompressLevel_9=9,   // Z_BEST_COMPRESSION
    ResourceCompressLevel_Max=9
} ResourceCompressLevel;

typedef struct ResourceCommonAttribute { // Size = 0x10
    // Same size as FFLiResourcePartsInfo, identical up to compressLevel.
    uint32_t offset;
    uint32_t size;
    uint32_t compressedSize;
    uint8_t compressLevel; // ResourceCompressLevel
    // FFLiResourcePartsInfo: u8 windowBits; // FFLiResourceWindowBits
    uint8_t memoryLevel;   // ResourceMemoryLevel / Verified but unused.
    // FFLiResourcePartsInfo: u8 strategy;   // FFLiResourceStrategy
    uint8_t pad[2];        // Unused padding for alignment.
} ResourceCommonAttribute;

// mii_ResourceShapeHeader.h

typedef enum ResourceShapeType {
    ResourceShapeType_Beard=0,
    ResourceShapeType_Faceline=1,
    ResourceShapeType_Mask=2,
    ResourceShapeType_HatForNormal=3,
    ResourceShapeType_HatForHeadWear=4,
    ResourceShapeType_ForeheadForNormal=5,
    ResourceShapeType_ForeheadForHeadWear=6,
    ResourceShapeType_HairForNormal=7,
    ResourceShapeType_HairForHeadWear=8,
    ResourceShapeType_Glass=9,
    ResourceShapeType_Nose=10,
    ResourceShapeType_Noseline=11,
    ResourceShapeType_End=12
} ResourceShapeType;

typedef struct ResourceShapeAttribute { // Size = 0x48
    uint32_t attributeOffset[5]; // AttributeType_End
    uint32_t attributeSize[5];   // AttributeType_End
    uint32_t indexOffset;        // IndexFormat_Uint16
    uint32_t indexSize;
    BoundingBox bounding;        // struct BoundingBox
} ResourceShapeAttribute;

typedef struct ResourceShapeHairTransform { // Size = 0x48
    // Same as FFLiResourceShapeHairTransform.
    Float3 frontTranslate;
    Float3 frontRotate;
    Float3 sideTranslate;
    Float3 sideRotate;
    Float3 topTranslate;
    Float3 topRotate;
} ResourceShapeHairTransform;

typedef struct ResourceShapeFacelineTransform { // Size = 0x24
    // Same as FFLiResourceShapeFacelineTransform.
    Float3 hairTranslate;
    Float3 noseTranslate;
    Float3 beardTranslate;
} ResourceShapeFacelineTransform;

typedef struct ShapeElement { // // ResourceShapeHeader::Element / Size = 0x58
    ResourceCommonAttribute common;
    ResourceShapeAttribute shape;
} ShapeElement;

typedef struct ResourceShapeHeader { // Size = 0x14dd4
    uint32_t signature;               // 0x5253464e / "NFSR"
    uint32_t version;                 // 1
    uint32_t fileSize;                // Includes size of header.
    uint32_t maxSize[12];             // ResourceShapeType_End
    uint32_t maxAlignment[12];        // Always 4
    ShapeElement beard[4];            // BeardType_ShapeCount
    ShapeElement faceline[12];        // FacelineType_End
    ShapeElement mask[12];            // FacelineType_End
    ShapeElement hatNormal[132];      // HairType_End
    ShapeElement hatCap[132];         // HairType_End
    ShapeElement foreheadNormal[132]; // HairType_End
    ShapeElement foreheadCap[132];    // HairType_End
    ShapeElement hairNormal[132];     // HairType_End
    ShapeElement hairCap[132];        // HairType_End
    ShapeElement glass[1]; 
    ShapeElement nose[18];            // NoseType_End
    ShapeElement noseline[18];        // NoseType_End
    ResourceShapeHairTransform hairTransform[132];        // HairType_End
    ResourceShapeFacelineTransform facelineTransform[12]; // FacelineType_End
} ResourceShapeHeader;

// mii_ResourceTextureHeader.h

typedef enum ResourceTextureType {
    ResourceTextureType_Hat=0,
    ResourceTextureType_Eye=1,
    ResourceTextureType_Eyebrow=2,
    ResourceTextureType_Beard=3,
    ResourceTextureType_Wrinkle=4,
    ResourceTextureType_Make=5,
    ResourceTextureType_Glass=6,
    ResourceTextureType_Mole=7,
    ResourceTextureType_Mouth=8,
    ResourceTextureType_Mustache=9,
    ResourceTextureType_Noseline=10,
    ResourceTextureType_End=11
} ResourceTextureType;

typedef struct ResourceTextureAttribute { // Size = 0xc
    // Same size as FFLiResourceTextureFooter, not identical.
    uint32_t alignment;
    uint16_t width;
    uint16_t height;
    uint8_t format;   // ResourceTextureFormat
    uint8_t mipCount;
    uint8_t tileMode; // ResourceTileMode
    uint8_t _pad[1];  // Unused padding for alignment.
} ResourceTextureAttribute;

typedef enum ResourceTileMode {
    ResourceTileMode_Optimal=0,
    ResourceTileMode_Linear=1,
    ResourceTileMode_End=2
} ResourceTileMode;

typedef enum ResourceTextureFormat {
    // Matches FFL.
    ResourceTextureFormat_R8_Unorm=0,
    ResourceTextureFormat_R8_B8_Unorm=1,
    ResourceTextureFormat_R8_G8_B8_A8_Unorm=2,
    ResourceTextureFormat_BC4_Unorm=3,
    ResourceTextureFormat_BC5_Unorm=4,
    ResourceTextureFormat_BC7_Unorm=5,
    ResourceTextureFormat_Astc4x4_Unorm=6,
    ResourceTextureFormat_End=7
} ResourceTextureFormat;

typedef struct TextureElement { // ResourceTextureHeader::Element / Size = 0x1c
    struct ResourceCommonAttribute common;
    struct ResourceTextureAttribute texture;
} TextureElement;

typedef struct ResourceTextureHeader { // Size = 0x2428
    uint32_t signature;          // 0x5254464e / "NFTR"
    uint32_t version;            // 1
    uint32_t fileSize;           // Includes size of header.
    uint32_t maxSize[11];        // ResourceTextureType_End
    uint32_t maxAlignment[11];   // Either 8 or 16.
    TextureElement hat[132];     // HairType_End
    TextureElement eye[62];      // EyeType_ResourceCount
    TextureElement eyebrow[24];  // EyebrowType_End
    TextureElement beard[2];     // BeardType_TextureCount
    TextureElement wrinkle[12];  // FacelineWrinkle_End
    TextureElement make[12];     // FacelineMake_End
    TextureElement glass[20];    // GlassType_End
    TextureElement mole[2];      // MoleType_End
    TextureElement mouth[37];    // MouthType_ResourceCount
    TextureElement mustache[6];  // MustacheType_End
    TextureElement noseline[18]; // NoseType_End
} ResourceTextureHeader;

// Not included:
// - ResourceIconBodyHeader
//    * Same contents as FFLBodyRes.dat, which uses the same header. It's unused anyway
// - ResourceVariableIconBodyHeader
//    * Just the Switch MiiEdit body model, half float, Pose00, can be parsed from bfres.

// Validation.

/*

// mii_ResourceCommonHeader.cpp
bool __thiscall nn::mii::detail::ResourceCommonAttribute::IsValid(ResourceCommonAttribute *this)
{
  if (this->size == 0) {
    return true;
  }
  if ((this->compressedSize != 0) && (this->compressLevel < 10)) {
    return (this->memoryLevel - 1) < ResourceMemoryLevel_Max;
  }
  return false;
}

// mii_ResourceShapeHeader.cpp

bool __thiscall nn::mii::detail::ResourceShapeHeader::IsValid(ResourceShapeHeader *this,size_t size)

{
  if ((this->signature == 0x5253464e) && (this->version == 1)) {
    return this->fileSize == size;
  }
  return false;
}

bool __thiscall
nn::mii::detail::ResourceShapeAttribute::IsValid(ResourceShapeAttribute *this,size_t rawSize)

{  
  uint32_t attributeSize = this->attributeSize[0];
  if (attributeSize != 0) {
    if (rawSize < this->attributeOffset[0] + attributeSize) {
      return false;
    }
    attributeSize = 1;
  }
  if (this->attributeSize[1] != 0) {
    if (rawSize < this->attributeOffset[1] + this->attributeSize[1]) {
      return false;
    }
    attributeSize = attributeSize + 1;
  }
  if (this->attributeSize[2] != 0) {
    if (rawSize < this->attributeOffset[2] + this->attributeSize[2]) {
      return false;
    }
    attributeSize = attributeSize + 1;
  }
  if (this->attributeSize[3] != 0) {
    if (rawSize < this->attributeOffset[3] + this->attributeSize[3]) {
      return false;
    }
    attributeSize = attributeSize + 1;
  }
  if (this->attributeSize[4] != 0) {
    if (rawSize < this->attributeOffset[4] + this->attributeSize[4]) {
      return false;
    }
    attributeSize = attributeSize + 1;
  }
  if (this->indexSize == 0) {
    return false;
  }
  return this->indexOffset + this->indexSize <= rawSize
      && attributeSize > 0;
}

bool __thiscall
nn::mii::detail::ResourceShapeAttribute::IsValidAttribute
          (ResourceShapeAttribute *this,AttributeType type)

{
  assert(type < AttributeType_End);
  return this->attributeSize[type] != 0;
}

// mii_ResourceTextureHeader.cpp

bool __thiscall
nn::mii::detail::ResourceTextureHeader::IsValid(ResourceTextureHeader *this,size_t size)

{
  if ((this->signature == 0x5254464e) && (this->version == 1)) {
    return this->fileSize == size;
  }
  return false;
}

bool __thiscall nn::mii::detail::ResourceTextureAttribute::IsValid(ResourceTextureAttribute *this)

{
  if ((this->alignment != 0) && 
      (this->format < ResourceTextureFormat_End) &&
      (this->width != 0) &&
      (this->height != 0) &&
      (this->mipCount != 0)))
  {
    return this->tileMode < ResourceTileMode_End;
  }
  return false;
}

*/