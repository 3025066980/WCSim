#include "polygon_mesh.hpp"

#include<glad/glad.h>
#include<GLFW/glfw3.h>

#include<fstream>
#include<assert.h>
#include<iostream>
#include<set>

#include "kdtree.hpp"
#include "ray.hpp"

#include "triangle.hpp"
#include "shader.hpp"
#include "camera.hpp"



PolygonMesh::PolygonMesh(std::map<float, std::map<float, float>> pattern_)
{
    shader_ = nullptr;
    model_ = glm::mat4(1.0f);

}

PolygonMesh::PolygonMesh(const std::string& path, Shader * shader) : tree_(nullptr),
                                                                     vao_(0),
                                                                      vbo_(0)
{
    shader_ = shader;
    model_ = glm::mat4(1.0f);

    LoadObj(path);
    tree_ = new KDTree(objects_);
    SetupMesh();
}

bool PolygonMesh::LoadObj(const std::string& path)
{
    /// move constructor to obj loader
    ///     /* Read the .obj file */
    if (path.substr(path.length() - 4) != ".obj") {
        assert("Cannot read PolygonMesh .obj");
        return false;
    } 
    else
    {
        std::ifstream input_file_stream(path, std::ios::in);

        std::vector<glm::vec3> vertices, normals;
        std::vector<glm::vec2> uvs;
        std::vector<unsigned int> vertex_indices, uv_indices, normal_indices;

        for (std::string buffer; input_file_stream >> buffer;) {
            //std::cout << "buffer" << buffer << std::endl;

            if (buffer == "v") {
                float x, y, z;
                input_file_stream >> x >> y >> z;
                vertices.push_back(glm::vec3(x, y, z));
            }
            else if (buffer == "vn") {
                float x, y, z;
                input_file_stream >> x >> y >> z;
                normals.push_back(glm::vec3(x, y, z));
            }
            else if (buffer == "vt") {
                float x, y;
                input_file_stream >> x >> y;
                uvs.push_back(glm::vec2(x, y));
            }
            else if (buffer == "f") {
                unsigned int vertex_index[3], uv_index[3], normal_index[3];
                for (auto i = 0; i < 3; ++i) {
                    char trash_char;
                    input_file_stream >> vertex_index[i] >> trash_char
                        >> uv_index[i] >> trash_char
                        >> normal_index[i];
                    --vertex_index[i];
                    --uv_index[i];
                    --normal_index[i];

                    vertices_.push_back({ vertices[vertex_index[i]], uvs[uv_index[i]], normals[normal_index[i]] });
                }

                // Build triangles for ray tracer
                const std::vector<glm::vec3> points = { vertices[vertex_index[0]], vertices[vertex_index[1]], vertices[vertex_index[2]] };
                //std::cout << "first normal" << normals[normal_index[0]].x << ", " << normals[normal_index[0]].y << ", " << normals[normal_index[0]].z << std::endl;
                //std::cout << "second normal" << normals[normal_index[1]].x << ", " << normals[normal_index[1]].y << ", " << normals[normal_index[1]].z << std::endl;
                //std::cout << "third normal" << normals[normal_index[2]].x << ", " << normals[normal_index[2]].y << ", " << normals[normal_index[2]].z << std::endl;

                objects_.push_back(new Triangle(points, normals[normal_index[0]]));
            }

        }

        input_file_stream.close();

    }
    return true;
}

void PolygonMesh::SetupMesh()
{
    std::cout << "DEBUG OBJ" << std::endl;

    std::cout << "Setting up mesh" << std::endl;
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);

    glBindVertexArray(vao_);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, vertices_.size() * sizeof(Vertex), &vertices_[0], GL_STATIC_DRAW);


    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
 
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

    glBindVertexArray(0);

}

bool PolygonMesh::IsHit(Ray &ray, float & t) const
{
    float temp_t;
    std::set<float> t_list;
    //return tree_->IsClosestHit(ray, t); ; /// to implement later, it hits but doesn't give correct t
    for (auto object : objects_) {
        if (object->IsHit(ray, temp_t)) {
            t_list.insert(temp_t);
        }
    }
    if (t_list.size() == 0) return false;
    //std::cout << "set size: " << t_list.size() << std::endl;
    t = *t_list.begin();
    return true;
}

bool PolygonMesh::IsHit(Ray& ray, float& t, Triangle *& hit_triangle) const
{
    float temp_t;
    std::set<std::pair<float, Triangle *>> t_list;

    //return tree_->IsHit(ray, t); ; /// to implement later, it hits but doesn't give correct t
    for (auto object : objects_) {
        Triangle * test_triangle = nullptr;
        if (object->IsHit(ray, temp_t, test_triangle)) {
            t_list.insert(std::pair{ temp_t , test_triangle});
        }
    }
    if (t_list.size() == 0) return false;
    //std::cout << "set size: " << t_list.size() << std::endl;
    t = std::get<0>(*t_list.begin());
    hit_triangle = std::get<1>(*t_list.begin());
    return true;
}

bool PolygonMesh::IsHit(Ray& ray, std::set<std::pair<float, Triangle*>> & hit_triangles) const
{
    float temp_t;
    //std::vector<float, Triangle *> hit_list;

    //return tree_->IsHit(ray, t); ; /// to implement later, it hits but doesn't give correct t
    for (auto object : objects_) {
        Triangle* test_triangle = nullptr;
        if (object->IsHit(ray, temp_t, test_triangle)) {
            hit_triangles.insert(std::pair{ temp_t, test_triangle });
        }
    }
    //std::cout << "hit triangles : " << hit_triangles.size() << std::endl;
    if (hit_triangles.size() == 0) return false;
    return true;
}

std::vector<const Triangle*> PolygonMesh::GetObjects()
{
    return std::vector<const Triangle*>(objects_);
}


void PolygonMesh::Draw() const {
    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLES, 0, vertices_.size());
    glBindVertexArray(0);
}