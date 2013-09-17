#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "common.h"

BEGIN_EXTENDED_CLASS(kexCamera, kexActor);
public:
                        kexCamera(void);
                        ~kexCamera(void);

    virtual void        LocalTick(void);

    void                SetupMatrices(void);
    void                UpdateAspect(void);
    const kexMatrix     &Projection(void) const { return projMatrix; }
    const kexMatrix     &ModelView(void) const { return modelMatrix; }

private:
    kexMatrix           projMatrix;
    kexMatrix           modelMatrix;
    float               zFar;
    float               zNear;
    float               fov;
    float               aspect;
    bool                bFixedFOV;
    bool                bLetterBox;
END_CLASS();

#endif
