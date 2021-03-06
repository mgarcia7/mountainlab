/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef DISKREADMDA_H
#define DISKREADMDA_H

#include "mda.h"

class DiskReadMdaPrivate;
/**
 * \class DiskReadMda
 * @brief Read-only access to a .mda file, especially useful for huge arrays that cannot be practically loaded into memory.
 *
 * See also Mda
 */
class DiskReadMda {
public:
    friend class DiskReadMdaPrivate;
    ///Constructor pointing to the .mda file specified by path (file name).
    DiskReadMda(const QString& path = "");
    ///Copy constructor
    DiskReadMda(const DiskReadMda& other);
    ///Constructor based on an in-memory array. This enables passing an Mda into a function that expects a DiskReadMda.
    DiskReadMda(const Mda& X);
    ///Destructor
    virtual ~DiskReadMda();
    ///Assignment operator
    void operator=(const DiskReadMda& other);

#ifdef QT_CORE_LIB
    ///Set the path (file name) of the .mda file to read.
    void setPath(const QString& file_path);
#endif
    ///Set the path (file name) of the .mda file to read.
    void setPath(const char* file_path);

    QString path();
    QString makePath();

    ///The first dimension of the array
    int N1() const;
    ///The second dimension of the array
    int N2() const;
    ///The third dimension of the array
    int N3() const;
    ///The fourth dimension of the array
    int N4() const;
    ///The fifth dimension of the array
    int N5() const;
    ///The sixth dimension of the array
    int N6() const;
    ///The product of N1() through N6()
    int totalSize() const;

    ///Retrieve a chunk of the vectorized data of size 1xN starting at position i
    bool readChunk(Mda& X, int i, int size) const;
    ///Retrieve a chunk of the vectorized data of size N1xN2 starting at position (i1,i2)
    bool readChunk(Mda& X, int i1, int i2, int size1, int size2) const;
    ///Retrieve a chunk of the vectorized data of size N1xN2xN3 starting at position (i1,i2,i3)
    bool readChunk(Mda& X, int i1, int i2, int i3, int size1, int size2, int size3) const;

    ///A slow method to retrieve the value at location i of the vectorized array for example value(3+4*N1())==value(3,4). Consider using readChunk() instead
    double value(int i) const;
    ///A slow method to retrieve the value at location (i1,i2) of the array. Consider using readChunk() instead
    double value(int i1, int i2) const;
    ///A slow method to retrieve the value at location (i1,i2,i3) of the array. Consider using readChunk() instead
    double value(int i1, int i2, int i3) const;

private:
    DiskReadMdaPrivate* d;
};

///Unit test
void diskreadmda_unit_test();

#endif // DISKREADMDA_H
