idf_component_register(
    SRCS "ground-station.c"
    INCLUDE_DIRS "."
    EMBED_TXTFILES amazonaws_com_root_cert.pem platzi_com_root_cert.pem
)
