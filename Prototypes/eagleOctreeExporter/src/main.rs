extern crate blender;
extern crate na;
//extern crate byteorder;
extern crate blender_mesh;

use std::fs;
use std::io::Write;
use std::thread;
use std::time::Duration;
use na::*;

//use byteorder::{LittleEndian, WriteBytesExt};

fn cubes<'a>(blend: &'a blender::File) -> Box<Iterator<Item = blender::Object> + 'a>{
    Box::new(blend.objects("Object")
        .filter(|obj| obj.name().unwrap().starts_with("OBCube")))
}

fn main() {
    let mut last_modification_date = None;
    loop{
        let modification_date = fs::metadata("assets/testVoxel.blend").unwrap().modified().unwrap();
        let reload = match last_modification_date{
            None => true,
            Some(date) => modification_date > date
        };
        if reload {
            println!("reloading");
            let blend = blender::File::load("assets/testVoxel.blend").unwrap();
            last_modification_date = Some(modification_date);
            let mut octree = fs::File::create("../../Projects/SceneVapor/bin/data/octree.txt").unwrap();
            for cube in cubes(&blend){
                let p = cube.get::<Vec3>("loc").unwrap();
                let mesh = blender_mesh::Mesh::parse(&cube.get_object("data").unwrap()).unwrap();
                let (min, max) = mesh.mvert.iter().fold((mesh.mvert[0].position.x,mesh.mvert[0].position.x), |(min,max), v|{
                    let mut min = min;
                    let mut max = max;
                    if v.position.x < min{
                        min = v.position.x;
                    }
                    if v.position.x > max{
                        max = v.position.x;
                    }
                    (min, max)
                });
                let side = max - min;
                let k =  1.;
                octree.write_fmt(format_args!("{} {} {} {}\n", p.x * k, p.y * k, p.z * k, side * k)).unwrap();
            }
        }
        thread::sleep(Duration::from_millis(16));
    }
}
