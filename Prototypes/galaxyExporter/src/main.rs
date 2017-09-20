extern crate blender;
extern crate na;
//extern crate byteorder;

use std::fs;
use std::io::Write;
use na::*;

//use byteorder::{LittleEndian, WriteBytesExt};

fn icospheres<'a>(blend: &'a blender::File) -> Box<Iterator<Item = blender::Object> + 'a>{
    Box::new(blend.objects("Object")
        .filter(|obj| obj.name().unwrap().starts_with("OBIcosphere")))
}

fn main() {
    let blend = blender::File::load("assets/galaxy_cycles_no_sponge_baked.blend").unwrap();
    let mut ply = fs::File::create("assets/galaxy.ply").unwrap();
    ply.write_fmt(format_args!("ply\n")).unwrap();
    ply.write_fmt(format_args!("format ascii 1.0\n")).unwrap();
    ply.write_fmt(format_args!("element vertex {}\n", icospheres(&blend).count())).unwrap();
    ply.write_fmt(format_args!("property float x\n")).unwrap();
    ply.write_fmt(format_args!("property float y\n")).unwrap();
    ply.write_fmt(format_args!("property float z\n")).unwrap();
    ply.write_fmt(format_args!("end_header\n")).unwrap();
    for mesh in icospheres(&blend){
        let p = mesh.get::<Vec3>("loc").unwrap();
        if p.x.abs() <= 1. && p.y.abs() <= 1. && p.z.abs() <= 1.{
            ply.write_fmt(format_args!("{} {} {}\n", p.x, p.y, p.z)).unwrap();
        }
        // ply.write_f32::<LittleEndian>(position[0]).unwrap();
        // ply.write_f32::<LittleEndian>(position[1]).unwrap();
        // ply.write_f32::<LittleEndian>(position[2]).unwrap();
        //println!("{:?} - {:?}", mesh.get::<[f32;3]>("loc").unwrap(), mesh.get::<[f32;3]>("size").unwrap());
    }
    ply.flush().unwrap();
}
