//
//  thickness.vsh
//  sub-surface scattering
//
//  Created by Jedd Haberstro on 6/2/11.
//  Copyright 2011 Student. All rights reserved.
//

void main() {
    float thickness = gl_FragCoord.z;
    if (gl_FrontFacing == false) {
        thickness = -thickness;
    }
    
    gl_FragColor = vec4(thickness, 0.0, 0.0, 1.0);
}