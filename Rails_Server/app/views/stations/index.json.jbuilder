json.array!(@stations) do |station|
  json.extract! station, :id, :name, :lat, :lon, :notes
  json.url station_url(station, format: :json)
end
