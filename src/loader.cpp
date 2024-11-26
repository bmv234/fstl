#include <future>

#include "loader.h"
#include "vertex.h"

Loader::Loader(QObject* parent, const QString& filename, bool is_reload)
    : QThread(parent), filename(filename), is_reload(is_reload)
{
    // Nothing to do here
}

void Loader::run()
{
    Mesh* mesh = load_stl();
    if (mesh)
    {
        if (mesh->empty())
        {
            emit error_empty_mesh();
            delete mesh;
        }
        else
        {
            emit got_mesh(mesh, is_reload);
            emit loaded_file(filename);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

void parallel_sort(QList<Vertex>& verts, int threads)
{
    if (threads < 2 || verts.size() < 2)
    {
        std::sort(verts.begin(), verts.end());
    }
    else
    {
        auto mid = verts.size() / 2;
        QList<Vertex> left(verts.begin(), verts.begin() + mid);
        QList<Vertex> right(verts.begin() + mid, verts.end());

        if (threads == 2)
        {
            auto future = std::async(parallel_sort, std::ref(left), threads / 2);
            parallel_sort(right, threads / 2);
            future.wait();
        }
        else
        {
            auto a = std::async(std::launch::async, parallel_sort, std::ref(left), threads / 2);
            auto b = std::async(std::launch::async, parallel_sort, std::ref(right), threads / 2);
            a.wait();
            b.wait();
        }

        // Merge the sorted halves back into verts
        std::merge(left.begin(), left.end(), right.begin(), right.end(), verts.begin());
    }
}

Mesh* mesh_from_verts(uint32_t tri_count, QList<Vertex>& verts)
{
    // Save indices as the second element in the array
    // (so that we can reconstruct triangle order after sorting)
    for (size_t i=0; i < tri_count*3; ++i)
    {
        verts[i].i = i;
    }

    // Check how many threads the hardware can safely support. This may return
    // 0 if the property can't be read so we should check for that too.
    auto threads = std::thread::hardware_concurrency();
    if (threads == 0)
    {
        threads = 8;
    }

    // Sort the set of vertices (to deduplicate)
    parallel_sort(verts, threads);

    // This vector will store triangles as sets of 3 indices
    std::vector<GLuint> indices(tri_count*3);

    // Go through the sorted vertex list, deduplicating and creating
    // an indexed geometry representation for the triangles.
    // Unique vertices are moved so that they occupy the first vertex_count
    // positions in the verts array.
    size_t vertex_count = 0;
    for (const auto& v : verts)
    {
        if (!vertex_count || v != verts[vertex_count-1])
        {
            verts[vertex_count++] = v;
        }
        indices[v.i] = vertex_count - 1;
    }
    verts.resize(vertex_count);

    std::vector<GLfloat> flat_verts;
    flat_verts.reserve(vertex_count*3);
    for (const auto& v : verts)
    {
        flat_verts.push_back(v.x);
        flat_verts.push_back(v.y);
        flat_verts.push_back(v.z);
    }

    return new Mesh(std::move(flat_verts), std::move(indices));
}

////////////////////////////////////////////////////////////////////////////////

Mesh* Loader::load_stl()
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
    {
        emit error_missing_file();
        return NULL;
    }

    qint64 file_size, file_size_old;
    file_size = file.size();
    do {
        file_size_old = file_size;
        QThread::usleep(100000);
        file_size = file.size();
    }
    while(file_size != file_size_old);

    // First, try to read the stl as an ASCII file
    if (file.read(5) == "solid")
    {
        file.readLine(); // skip solid name
        const auto line = file.readLine().trimmed();
        if (line.startsWith("facet") ||
            line.startsWith("endsolid"))
        {
            file.seek(0);
            return read_stl_ascii(file);
        }
        // Otherwise, this STL is a binary stl but contains 'solid' as
        // the first five characters.  This is a bad life choice, but
        // we can gracefully handle it by falling through to the binary
        // STL reader below.
    }

    file.seek(0);
    return read_stl_binary(file);
}

Mesh* Loader::read_stl_binary(QFile& file)
{
    QDataStream data(&file);
    data.setByteOrder(QDataStream::LittleEndian);
    data.setFloatingPointPrecision(QDataStream::SinglePrecision);

    // Load the triangle count from the .stl file
    file.seek(80);
    uint32_t tri_count;
    data >> tri_count;

    // Verify that the file is the right size
    if (file.size() != 84 + tri_count*50)
    {
        emit error_bad_stl();
        return NULL;
    }

    // Extract vertices into an array of xyz, unsigned pairs
    QList<Vertex> verts;
    verts.reserve(tri_count*3);

    // Dummy array, because readRawData is faster than skipRawData
    std::unique_ptr<uint8_t[]> buffer(new uint8_t[tri_count * 50]);
    data.readRawData((char*)buffer.get(), tri_count * 50);

    // Store vertices in the array, processing one triangle at a time.
    auto b = buffer.get() + 3 * sizeof(float);
    for (uint32_t i = 0; i < tri_count * 3; i += 3)
    {
        // Load vertex data from .stl file into vertices
        for (unsigned j = 0; j < 3; ++j)
        {
            float coords[3];
            qFromLittleEndian<float>(b, 3, coords);
            verts.append(Vertex(coords[0], coords[1], coords[2]));
            b += 3 * sizeof(float);
        }

        // Skip face attribute and next face's normal vector
        b += 3 * sizeof(float) + sizeof(uint16_t);
    }

    return mesh_from_verts(tri_count, verts);
}

Mesh* Loader::read_stl_ascii(QFile& file)
{
    file.readLine();
    uint32_t tri_count = 0;
    QList<Vertex> verts;

    bool okay = true;
    while (!file.atEnd() && okay)
    {
        const auto line = file.readLine().simplified();
        if (line.startsWith("endsolid"))
        {
            break;
        }
        else if (!line.startsWith("facet normal") ||
                 !file.readLine().simplified().startsWith("outer loop"))
        {
            okay = false;
            break;
        }

        for (int i=0; i < 3; ++i)
        {
            auto line = file.readLine().simplified().split(' ');
            if (line[0] != "vertex")
            {
                okay = false;
                break;
            }
            const float x = line[1].toFloat(&okay);
            const float y = line[2].toFloat(&okay);
            const float z = line[3].toFloat(&okay);
            verts.append(Vertex(x, y, z));
        }
        if (!file.readLine().trimmed().startsWith("endloop") ||
            !file.readLine().trimmed().startsWith("endfacet"))
        {
            okay = false;
            break;
        }
        tri_count++;
    }

    if (okay)
    {
        return mesh_from_verts(tri_count, verts);
    }
    else
    {
        emit error_bad_stl();
        return NULL;
    }
}
