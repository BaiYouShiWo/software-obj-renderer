#include "linear.h"
#include "canvas.h"
#include "graph.h"
#include "render.h"
#include "camera.h"
#include "timer.h"
#include "model.h"
#include "gui.h"
#include "inputmanger.h"
#include <filesystem>
namespace fs = std::filesystem;

int main(int argc, char* argv[]){

    int SCREEN_WIDTH = 1920;
    int SCREEN_HEIGHT = 1080;

    std::string model_path;
    std::string texture_path;

    if(argc > 1){
        fs::path possible_path = argv[1];
        if (!fs::exists(possible_path)) {
            std::cout << "Cannot open file: "<< possible_path << "\n";
            getchar();
            return -1;
        }
        model_path = possible_path.generic_string();
    }else{
        std::cout << "Obj file path(press Enter to use default path): ";
        std::string input_line;
        std::getline(std::cin, input_line);

        if (input_line.empty()) {
            std::cout << "No model file input\n";
             getchar();
             return -1;
        } else {
            model_path = input_line;
        }

        if (!fs::exists(model_path)) {
            std::cerr << "Error: model file doesn't exist: " << model_path << std::endl;
            getchar();
            return -1;
        }

        std::cout << "Texture path(Png only, press Enter to skip): ";
        std::getline(std::cin, input_line);
        if (input_line.empty()) {
            texture_path = "";
            std::cout << "Render obj only." << std::endl;
        } else {
            texture_path = input_line;
        }

    }

    model::Model render_model = model::loadModel(model_path);
    texture::Texture render_texture = texture::loadTexture(texture_path);

    WindowsInputManager inputManager(GetConsoleWindow());
    Camera camera((float)PI/2.f, (float)SCREEN_WIDTH/(float)SCREEN_HEIGHT, 1.f, 20.0f, 
                 vec3(0,0,4), vec3(0,0,0), vec3(0,-1,0));

    Picture image(SCREEN_WIDTH, SCREEN_HEIGHT, 3);
    Matrix depthBuffer(SCREEN_HEIGHT, SCREEN_WIDTH);

    RaylibPictureRenderer viewer(SCREEN_WIDTH, SCREEN_HEIGHT, "Raylib Picture Viewer");
    viewer.initialize(image);

    RenderTimer timer(60);
    std::vector<vec4> transfromed_vertices(render_model.transfromed_vertices.size());
    Matrix vertex(4,1);

    bool running = true;
    while(running){
        timer.tick();

        auto dt = timer.getDeltaTime();
        auto tt = timer.getTotalTime();

        Matrix View = camera.view_matrix();
        Matrix Perspective = camera.perspective_matrix();
        Matrix MVP = Perspective * View;
        

        //for(int i=0; i<render_model.vertices.size(); i++){
        //    
        //    vertex(0,0) = render_model.vertices[i].x;
        //    vertex(1,0) = render_model.vertices[i].y;
        //    vertex(2,0) = render_model.vertices[i].z;
        //    vertex(3,0) = 1.0f;

        //    Matrix transformed = MVP * vertex;
        //    float w = transformed(3,0);
        //    render_model.transfromed_vertices[i] = vec4(
        //    transformed(0,0),
        //    transformed(1,0),
        //    transformed(2,0),
        //    transformed(3,0)
        //    );
        //}

         transform_batch_aos(render_model.transfromed_vertices.data(),MVP.data().data(),
         render_model.mesh.x.data(),render_model.mesh.y.data(),render_model.mesh.z.data(),
         render_model.mesh.x.size());
        

        render(render_model, render_texture, image, depthBuffer);

        inputManager.update();
        camera.update(inputManager, dt);

        viewer.updateTexture(image);
        viewer.draw();

        timer.waitIfNeeded();
    }
    system("pause");
    return 0;

}