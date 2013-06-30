void release_image_data(image_data_s * image_data)
{
	free(image_data->data);
	free(image_data);
}

image_data_s * scale_image_data_with_factor(image_data_s *source_image_data, float factor, image_data_s *dest_image_data)
{
}

image_data_s * scale_image_data(image_data_s *source_image_data, int new_width, int new_height, image_data_s *dest_image_data)
{
	
}
