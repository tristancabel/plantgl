 # -Geom File-
(#--------------------------------------------------
 #  
 #  File : D:/Fred/Mes documents/Develop/AMAPmod/databases/GEOMFiles/icosa.geom
 #  
 #  A GEOM file generated with GEOM library
 #  Published under the GNU General Public Licence. 
 #  
 # --------------------------------------------------
 #)

FaceSet GEOM_17689052 { 
    PointList [ 
        <0,0,2>, 
        <1.78885,0,0.894427>, 
        <0.552786,1.7013,0.894427>, 
        <-1.44721,1.05146,0.894427>, 
        <-1.44721,-1.05146,0.894427>, 
        <0.552786,-1.7013,0.894427>, 
        <1.44721,1.05146,-0.894427>, 
        <-0.552786,1.7013,-0.894427>, 
        <-1.78885,0,-0.894427>, 
        <-0.552786,-1.7013,-0.894427>, 
        <1.44721,-1.05146,-0.894427>, 
        <0,0,-2>
    ]
    IndexList [ 
        [2,0,1], 
        [3,0,2], 
        [4,0,3], 
        [5,0,4], 
        [1,0,5], 
        [2,1,6], 
        [7,2,6], 
        [3,2,7], 
        [8,3,7], 
        [4,3,8], 
        [9,4,8], 
        [5,4,9], 
        [10,5,9], 
        [6,1,10], 
        [1,5,10], 
        [6,11,7], 
        [7,11,8], 
        [8,11,9], 
        [9,11,10], 
        [10,11,6]
    ]
    NormalPerVertex True
}

Material DEFAULT_MATERIAL { 
}

Shape SHAPE_17689052 { 
    Geometry  GEOM_17689052
    Appearance  DEFAULT_MATERIAL
}
